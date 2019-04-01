/// <reference types="Cypress" />

context('Main', () => {
  beforeEach(() => {
    cy.visit('/')
  })

  it('check webpack output', () => {
    cy.get('#router__content')
      .should('have.text', 'Hello, World!');
  })
})
