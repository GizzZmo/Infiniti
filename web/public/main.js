let authToken = localStorage.getItem('authToken') || '';
let challengeToken = '';

const elements = {
  output: document.getElementById('output'),
  feedback: document.getElementById('feedback'),
  qrWrap: document.getElementById('qrWrap'),
  regEmail: document.getElementById('regEmail'),
  regPassword: document.getElementById('regPassword'),
  loginEmail: document.getElementById('loginEmail'),
  loginPassword: document.getElementById('loginPassword'),
  loginOtp: document.getElementById('loginOtp'),
  loginRecovery: document.getElementById('loginRecovery'),
  forgotEmail: document.getElementById('forgotEmail'),
  enableOtp: document.getElementById('enableOtp'),
  disableOtp: document.getElementById('disableOtp'),
  registerBtn: document.getElementById('registerBtn'),
  loginBtn: document.getElementById('loginBtn'),
  submit2faBtn: document.getElementById('submit2faBtn'),
  resendVerifyBtn: document.getElementById('resendVerifyBtn'),
  forgotBtn: document.getElementById('forgotBtn'),
  setup2faBtn: document.getElementById('setup2faBtn'),
  enable2faBtn: document.getElementById('enable2faBtn'),
  disable2faBtn: document.getElementById('disable2faBtn'),
  refreshSessionBtn: document.getElementById('refreshSessionBtn'),
  signOutBtn: document.getElementById('signOutBtn'),
  sessionCard: document.getElementById('sessionCard'),
  sessionHeadline: document.getElementById('sessionHeadline'),
  sessionSummary: document.getElementById('sessionSummary'),
  sessionEmail: document.getElementById('sessionEmail'),
  sessionVerified: document.getElementById('sessionVerified'),
  sessionTwoFactor: document.getElementById('sessionTwoFactor'),
  sessionAccess: document.getElementById('sessionAccess'),
};

const passwordHelp = 'Use at least 8 characters with uppercase, lowercase, and a number.';

function setFeedback(kind, message) {
  elements.feedback.className = `feedback ${kind} show`;
  elements.feedback.textContent = message;
}

function log(data) {
  elements.output.textContent = typeof data === 'string' ? data : JSON.stringify(data, null, 2);
}

function parseResponse(text) {
  try {
    return JSON.parse(text);
  } catch {
    return text;
  }
}

function formatError(error) {
  const payload = parseResponse(error.message || '');
  if (payload && typeof payload === 'object' && payload.error) {
    return payload.error;
  }
  if (typeof payload === 'string' && payload) {
    return payload;
  }
  return 'Request failed.';
}

function sanitizeForLog(data) {
  if (!data || typeof data !== 'object') return data;
  if (Array.isArray(data)) return data.map(sanitizeForLog);
  const copy = {};
  Object.keys(data).forEach((key) => {
    if (key === 'token') copy[key] = '[stored securely in browser]';
    else if (key === 'challengeToken') copy[key] = '[challenge issued]';
    else if (key === 'secret') copy[key] = '[shown in setup panel]';
    else if (key === 'qrCodeDataUrl') copy[key] = '[rendered in setup panel]';
    else copy[key] = sanitizeForLog(data[key]);
  });
  return copy;
}

function rememberToken(token) {
  authToken = token || '';
  if (authToken) localStorage.setItem('authToken', authToken);
  else localStorage.removeItem('authToken');
}

function clearQrSetup() {
  elements.qrWrap.replaceChildren();
}

function updateSessionCard(variant, headline, summary) {
  elements.sessionCard.className = `session-card ${variant}`;
  elements.sessionHeadline.textContent = headline;
  elements.sessionSummary.textContent = summary;
}

function updateSessionUi(user) {
  if (!user) {
    updateSessionCard('warning', 'No active session', 'Sign in to access KYC, transactions, favourites, and admin tools.');
    elements.sessionEmail.textContent = 'Not signed in';
    elements.sessionVerified.textContent = 'Unknown';
    elements.sessionTwoFactor.textContent = 'Unknown';
    elements.sessionAccess.textContent = 'Standard';
    return;
  }

  const verified = user.emailVerified ? 'Verified' : 'Action needed';
  const twoFactor = user.twoFactorEnabled ? 'Enabled' : 'Not enabled';
  const access = user.isAdmin ? 'Admin' : 'Standard';
  const variant = !user.emailVerified ? 'warning' : user.twoFactorEnabled ? 'success' : 'danger';
  const headline = user.emailVerified ? `Signed in as ${user.email}` : `Signed in, verify ${user.email}`;
  const summary = user.emailVerified
    ? user.twoFactorEnabled
      ? 'Account is verified and protected with 2FA.'
      : 'Account works, but enabling 2FA will improve security.'
    : 'Verify your email before using protected transaction flows.';

  updateSessionCard(variant, headline, summary);
  elements.sessionEmail.textContent = user.email;
  elements.sessionVerified.textContent = verified;
  elements.sessionTwoFactor.textContent = twoFactor;
  elements.sessionAccess.textContent = access;
}

async function api(url, options = {}) {
  const headers = { ...(options.headers || {}) };
  const sentAuth = Boolean(authToken) && !options.skipAuth;
  if (sentAuth) headers.Authorization = 'Token ' + authToken;
  if (!headers['Content-Type'] && !(options.body instanceof FormData)) {
    headers['Content-Type'] = 'application/json';
  }

  const response = await fetch(url, { ...options, headers });
  const text = await response.text();
  const data = parseResponse(text);

  if (!response.ok) {
    if (response.status === 401 && sentAuth) {
      rememberToken('');
      updateSessionUi(null);
    }
    throw new Error(typeof data === 'string' ? data : JSON.stringify(data));
  }

  return data;
}

async function runAction(button, action) {
  const original = button.textContent;
  button.disabled = true;
  button.textContent = 'Working...';
  try {
    return await action();
  } finally {
    button.disabled = false;
    button.textContent = original;
  }
}

function readTrimmedValue(input) {
  return input.value.trim();
}

function validateEmail(value) {
  return /\S+@\S+\.\S+/.test(value);
}

function validatePassword(value) {
  return value.length >= 8 && /[a-z]/.test(value) && /[A-Z]/.test(value) && /\d/.test(value);
}

function clearSession() {
  rememberToken('');
  challengeToken = '';
  clearQrSetup();
  updateSessionUi(null);
}

function renderQrSetup(setup) {
  clearQrSetup();

  const image = document.createElement('img');
  image.src = setup.qrCodeDataUrl;
  image.alt = '2FA QR code';
  image.width = 180;

  const secret = document.createElement('p');
  secret.textContent = `Secret: ${setup.secret}`;

  const note = document.createElement('p');
  note.className = 'helper-text';
  note.textContent = 'Scan the QR code in your authenticator app, then confirm with a fresh OTP below.';

  elements.qrWrap.append(image, secret, note);
}

async function refreshSession() {
  if (!authToken) {
    updateSessionUi(null);
    return null;
  }

  try {
    const data = await api('/api/auth/me');
    updateSessionUi(data.user);
    return data.user;
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
    return null;
  }
}

elements.registerBtn.onclick = () => runAction(elements.registerBtn, async () => {
  const email = readTrimmedValue(elements.regEmail).toLowerCase();
  const password = elements.regPassword.value;

  if (!validateEmail(email)) {
    setFeedback('error', 'Enter a valid registration email address.');
    log({ error: 'Invalid registration email.' });
    return;
  }
  if (!validatePassword(password)) {
    setFeedback('error', passwordHelp);
    log({ error: passwordHelp });
    return;
  }

  try {
    const data = await api('/api/auth/register', {
      method: 'POST',
      skipAuth: true,
      body: JSON.stringify({ email, password }),
    });
    elements.loginEmail.value = email;
    setFeedback('success', 'Account created. Check your inbox, then sign in once your email is verified.');
    log(sanitizeForLog(data));
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.loginBtn.onclick = () => runAction(elements.loginBtn, async () => {
  const email = readTrimmedValue(elements.loginEmail).toLowerCase();
  const password = elements.loginPassword.value;

  if (!validateEmail(email)) {
    setFeedback('error', 'Enter the email address tied to your account.');
    log({ error: 'Invalid login email.' });
    return;
  }
  if (!password) {
    setFeedback('error', 'Enter your password to continue.');
    log({ error: 'Missing password.' });
    return;
  }

  try {
    const data = await api('/api/auth/login', {
      method: 'POST',
      skipAuth: true,
      body: JSON.stringify({ email, password }),
    });
    if (data.requires2fa) {
      challengeToken = data.challengeToken;
      setFeedback('info', '2FA required. Enter an authenticator code or a recovery code to finish sign-in.');
      log(sanitizeForLog({ message: '2FA challenge created.', challengeToken: data.challengeToken }));
      return;
    }

    rememberToken(data.token);
    challengeToken = '';
    elements.loginOtp.value = '';
    elements.loginRecovery.value = '';
    updateSessionUi(data.user);
    setFeedback('success', 'Signed in. Your shared session is ready for the linked tools.');
    log(sanitizeForLog(data));
    await refreshSession();
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.submit2faBtn.onclick = () => runAction(elements.submit2faBtn, async () => {
  const otp = readTrimmedValue(elements.loginOtp);
  const recoveryCode = readTrimmedValue(elements.loginRecovery);

  if (!challengeToken) {
    setFeedback('error', 'Start sign-in first so the portal can issue a 2FA challenge.');
    log({ error: 'Missing 2FA challenge.' });
    return;
  }
  if (!otp && !recoveryCode) {
    setFeedback('error', 'Enter either an authenticator code or a recovery code.');
    log({ error: 'Missing OTP or recovery code.' });
    return;
  }

  try {
    const data = await api('/api/auth/login/2fa', {
      method: 'POST',
      skipAuth: true,
      body: JSON.stringify({ challengeToken, otp, recoveryCode }),
    });
    rememberToken(data.token);
    challengeToken = '';
    elements.loginOtp.value = '';
    elements.loginRecovery.value = '';
    updateSessionUi(data.user);
    setFeedback('success', '2FA sign-in complete.');
    log(sanitizeForLog(data));
    await refreshSession();
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.resendVerifyBtn.onclick = () => runAction(elements.resendVerifyBtn, async () => {
  if (!authToken) {
    setFeedback('error', 'Sign in before requesting a new verification email.');
    log({ error: 'No active session.' });
    return;
  }

  try {
    const data = await api('/api/auth/verify-email/resend', { method: 'POST' });
    setFeedback('success', 'Verification email sent if your account still needs one.');
    log(sanitizeForLog(data));
    await refreshSession();
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.forgotBtn.onclick = () => runAction(elements.forgotBtn, async () => {
  const email = readTrimmedValue(elements.forgotEmail).toLowerCase();
  if (!validateEmail(email)) {
    setFeedback('error', 'Enter the email address that should receive the reset link.');
    log({ error: 'Invalid reset email.' });
    return;
  }

  try {
    const data = await api('/api/auth/forgot-password', {
      method: 'POST',
      skipAuth: true,
      body: JSON.stringify({ email }),
    });
    setFeedback('success', 'If the account exists, a password reset email has been sent.');
    log(sanitizeForLog(data));
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.setup2faBtn.onclick = () => runAction(elements.setup2faBtn, async () => {
  if (!authToken) {
    setFeedback('error', 'Sign in before starting 2FA setup.');
    log({ error: 'No active session.' });
    return;
  }

  try {
    const setup = await api('/api/auth/2fa/setup', { method: 'POST' });
    renderQrSetup(setup);
    setFeedback('info', 'QR code ready. Scan it and confirm with a fresh OTP.');
    log(sanitizeForLog(setup));
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.enable2faBtn.onclick = () => runAction(elements.enable2faBtn, async () => {
  const otp = readTrimmedValue(elements.enableOtp);
  if (!otp) {
    setFeedback('error', 'Enter the authenticator code shown in your app to enable 2FA.');
    log({ error: 'Missing enable 2FA OTP.' });
    return;
  }

  try {
    const data = await api('/api/auth/2fa/enable', {
      method: 'POST',
      body: JSON.stringify({ otp }),
    });
    elements.enableOtp.value = '';
    setFeedback('success', '2FA enabled. Save your recovery codes in a secure place.');
    log(sanitizeForLog(data));
    await refreshSession();
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.disable2faBtn.onclick = () => runAction(elements.disable2faBtn, async () => {
  const otp = readTrimmedValue(elements.disableOtp);
  if (!otp) {
    setFeedback('error', 'Enter the current authenticator code to disable 2FA.');
    log({ error: 'Missing disable 2FA OTP.' });
    return;
  }

  try {
    const data = await api('/api/auth/2fa/disable', {
      method: 'POST',
      body: JSON.stringify({ otp }),
    });
    elements.disableOtp.value = '';
    clearQrSetup();
    setFeedback('success', '2FA disabled for the current account.');
    log(sanitizeForLog(data));
    await refreshSession();
  } catch (error) {
    setFeedback('error', formatError(error));
    log({ error: formatError(error) });
  }
});

elements.refreshSessionBtn.onclick = () => runAction(elements.refreshSessionBtn, async () => {
  if (!authToken) {
    updateSessionUi(null);
    setFeedback('info', 'No session token is stored right now.');
    log({ message: 'No active session.' });
    return;
  }

  const user = await refreshSession();
  if (user) {
    setFeedback('success', 'Session refreshed.');
    log({ user });
  }
});

elements.signOutBtn.onclick = () => runAction(elements.signOutBtn, async () => {
  clearSession();
  setFeedback('info', 'Signed out on this device. Linked pages will need a new sign-in.');
  log({ success: true, message: 'Session cleared from local storage.' });
});

updateSessionUi(null);
if (authToken) {
  refreshSession()
    .then((user) => {
      if (user) {
        setFeedback('success', 'Existing session restored from local storage.');
        log({ user });
      }
    });
}
