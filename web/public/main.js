let authToken = localStorage.getItem('authToken') || '';
let challengeToken = '';

const output = document.getElementById('output');
const api = async (url, options = {}) => {
  const headers = { ...(options.headers || {}) };
  if (authToken) headers.Authorization = 'Token ' + authToken;
  if (!headers['Content-Type'] && !(options.body instanceof FormData)) {
    headers['Content-Type'] = 'application/json';
  }
  const response = await fetch(url, { ...options, headers });
  const text = await response.text();
  let data;
  try { data = JSON.parse(text); } catch { data = text; }
  if (!response.ok) throw new Error(JSON.stringify(data));
  return data;
};

function log(data) {
  output.textContent = typeof data === 'string' ? data : JSON.stringify(data, null, 2);
}

document.getElementById('registerBtn').onclick = async () => {
  try {
    const data = await api('/api/auth/register', {
      method: 'POST',
      body: JSON.stringify({
        email: document.getElementById('regEmail').value,
        password: document.getElementById('regPassword').value,
      }),
    });
    log(data);
  } catch (err) { log(err.message); }
};

document.getElementById('loginBtn').onclick = async () => {
  try {
    const data = await api('/api/auth/login', {
      method: 'POST',
      body: JSON.stringify({
        email: document.getElementById('loginEmail').value,
        password: document.getElementById('loginPassword').value,
      }),
    });
    if (data.requires2fa) {
      challengeToken = data.challengeToken;
      log('2FA required. Enter OTP or recovery code and submit 2FA.');
    } else {
      authToken = data.token;
      localStorage.setItem('authToken', authToken);
      log(data);
    }
  } catch (err) { log(err.message); }
};

document.getElementById('submit2faBtn').onclick = async () => {
  try {
    const data = await api('/api/auth/login/2fa', {
      method: 'POST',
      body: JSON.stringify({
        challengeToken,
        otp: document.getElementById('loginOtp').value,
        recoveryCode: document.getElementById('loginRecovery').value,
      }),
    });
    authToken = data.token;
    localStorage.setItem('authToken', authToken);
    log(data);
  } catch (err) { log(err.message); }
};

document.getElementById('resendVerifyBtn').onclick = async () => {
  try { log(await api('/api/auth/verify-email/resend', { method: 'POST' })); }
  catch (err) { log(err.message); }
};

document.getElementById('forgotBtn').onclick = async () => {
  try {
    log(await api('/api/auth/forgot-password', {
      method: 'POST',
      body: JSON.stringify({ email: document.getElementById('forgotEmail').value }),
    }));
  } catch (err) { log(err.message); }
};

document.getElementById('setup2faBtn').onclick = async () => {
  try {
    const setup = await api('/api/auth/2fa/setup', { method: 'POST' });
    document.getElementById('qrWrap').innerHTML = `<img alt="2FA QR" src="${setup.qrCodeDataUrl}" width="180" /><p>Secret: ${setup.secret}</p>`;
    log({ message: 'Scan the QR and confirm with OTP.' });
  } catch (err) { log(err.message); }
};

document.getElementById('enable2faBtn').onclick = async () => {
  try {
    log(await api('/api/auth/2fa/enable', {
      method: 'POST',
      body: JSON.stringify({ otp: document.getElementById('enableOtp').value }),
    }));
  } catch (err) { log(err.message); }
};

document.getElementById('disable2faBtn').onclick = async () => {
  try {
    log(await api('/api/auth/2fa/disable', {
      method: 'POST',
      body: JSON.stringify({ otp: document.getElementById('disableOtp').value }),
    }));
  } catch (err) { log(err.message); }
};
