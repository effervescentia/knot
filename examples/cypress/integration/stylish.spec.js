/// <reference types="Cypress" />

context('Stylish', () => {
  beforeEach(() => {
    cy.visit('/');

    // cy.get('#route--stylish').click();
  });

  it('has root styles', () => {
    cy.get('#stylish')
      .should('have.css', 'height', '200px')
      .should('have.css', 'background-color', 'rgb(0, 0, 128)');
  });

  it('has child styles', () => {
    cy.get('#stylish__child--top')
      .should('have.css', 'height', '100px')
      .should('have.css', 'width', '200px')
      .should('have.css', 'background-color', 'rgb(255, 192, 203)');

    cy.get('#stylish__child--bottom')
      .should('have.css', 'height', '100px')
      .should('have.css', 'width', '200px')
      .should('have.css', 'background-color', 'rgb(238, 130, 238)');
  });
});
