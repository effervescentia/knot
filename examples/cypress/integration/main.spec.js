/// <reference types="Cypress" />

context('Main', () => {
  beforeEach(() => {
    cy.visit('/');
  });

  it.skip('has routes', () => {
    const routes = ['hello world', 'toggle', 'calculator', 'stylish'];

    cy.get('#route-list')
      .children()
      .should('have.length', 4)
      .each(($el, index) => cy.wrap($el).should('have.text', routes[index]));
  });

  it.skip('visit greeting route', () => {
    const isGreetingContent = () =>
      cy.get('#router__content').should('have.text', 'Hello, World!');

    isGreetingContent();

    tryRoute('greeting');

    isGreetingContent();
  });

  it.skip('visit toggle route', () => {
    tryRoute('toggle');

    cy.get('#button--toggle').should('be.visible');
  });

  it.skip('visit calculator route', () => {
    tryRoute('calculator');

    cy.get('#calculator').should('be.visible');
  });

  it.skip('visit stylish route', () => {
    tryRoute('stylish');

    cy.get('#stylish').should('be.visible');
  });
});

function isRoute(name) {
  return cy.hash().should('eq', `#${name}`);
}

function tryRoute(name) {
  cy.get(`#route--${name}`).click();

  return isRoute(name);
}
