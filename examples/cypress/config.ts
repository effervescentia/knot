import { defineConfig } from 'cypress';

export default defineConfig({
  e2e: {
    baseUrl: 'http://localhost:1337',
    supportFolder: '../cypress/support',
    supportFile: '../cypress/support/e2e.js',
    specPattern: '../cypress/e2e/**/*.cy.{js,jsx,ts,tsx}',
  },
});
