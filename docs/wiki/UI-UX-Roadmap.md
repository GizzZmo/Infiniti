# UI/UX Roadmap

This roadmap focuses the web platform on five pillars: **usability**, **clarity**, **authentication**, **performance**, and **security**.

> Status in this branch: **Stage 1 / Phase 2 complete**

---

## Stage 1 — Trustworthy Foundations

### Phase 1 — Auth Portal Clarity *(complete in this branch)*

- Rework the landing page into clear task-based sections for registration, sign-in, recovery, and 2FA.
- Add visible session state so users can confirm whether they are signed in, verified, protected by 2FA, or have admin access.
- Improve form guidance with labels, helper text, inline requirements, and action-specific feedback.
- Prevent duplicate submissions with loading states and disabled actions during requests.
- Reduce accidental token exposure by keeping tokens in storage for navigation while showing safer success summaries in the UI.
- Replace unsafe HTML injection in the 2FA setup flow with DOM-based rendering.

### Phase 2 — Shared Navigation + Workflow Consistency *(complete in this branch)*

- Apply one shared visual shell across account, KYC, transactions, favourites, and admin pages.
- Standardise feedback states, empty states, error handling, and primary/secondary actions.
- Add persistent navigation cues so users always know the next safe step.

### Phase 3 — Responsive Data Flows

- Improve mobile layout, spacing, and form density for smaller screens.
- Add progressive disclosure for advanced options to reduce cognitive load.
- Streamline long JSON outputs into readable summaries and structured tables.

---

## Stage 2 — Operational UX

### Phase 1 — KYC and Admin Review

- Turn upload and review screens into guided workflows with requirements, file constraints, and review outcomes.
- Surface security-sensitive states clearly, including approval status, rejection reasons, and audit context.

### Phase 2 — Transactions and Favourites

- Replace raw payload dumps with filter summaries, result counts, and readable transaction rows.
- Improve performance with smaller initial payloads, clearer export flow, and more efficient refresh behaviour.

---

## Stage 3 — Security + Performance Hardening

### Phase 1 — Authentication Hardening

- Add safer session lifecycle controls such as explicit sign-out affordances and clearer invalid-session recovery.
- Review token handling ergonomics and move toward less error-prone client session patterns.

### Phase 2 — Frontend Performance

- Reduce avoidable DOM work, reuse state helpers, and minimise repeated rendering paths.
- Expand packaging checks and screenshots to cover the improved critical flows.

### Phase 3 — Confidence and Observability

- Add focused UI regression coverage for authentication and account-critical flows.
- Track key UX health signals such as failed login loops, abandoned 2FA setup, and repeated recovery attempts.
