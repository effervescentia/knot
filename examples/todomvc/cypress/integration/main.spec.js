/// <reference types="Cypress" />

context('Main', () => {
  beforeEach(() => {
    cy.visit('/')
  })

  it('check webpack output', () => {
    cy.get('#app')
      .should('have.text', 'Hello, World!');
  })
})