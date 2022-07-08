/// <reference types="Cypress" />

context('Toggle', () => {
  beforeEach(() => {
    cy.visit('/');

    cy.get('#route--toggle').click();
  });

  it.skip('has initial value', () => {
    cy.get('#router__content').should('have.text', 'toggle:ON');
  });

  it.skip('updates value', () => {
    cy.get('#button--toggle').click();

    cy.get('#router__content').should('have.text', 'toggle:OFF');
  });
});
