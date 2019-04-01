/// <reference types="Cypress" />

context('Main', () => {
  // beforeEach(() => {
  // });

  it('check webpack output', () => {
    cy.visit('/');

    cy.get('#router__content')
      .should('have.text', 'Hello, World!');
  });
});
