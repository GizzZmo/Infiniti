require('dotenv').config();
const { PORT } = require('./config');
const { app } = require('./app');

app.listen(PORT, () => {
  // eslint-disable-next-line no-console
  console.log(`Infiniti web platform listening on http://localhost:${PORT}`);
});
