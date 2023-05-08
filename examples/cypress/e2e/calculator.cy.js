/// <reference types="Cypress" />

context('Calculator', () => {
  beforeEach(() => {
    cy.visit('/');

    cy.get('#route--calculator').click();
  });

  it('has initial values', () => {
    cy.get('#calculator__result').should('have.text', '0');
    cy.get('#calculator__input').should('have.text', '0');
  });

  it('updates values', () => {
    cy.get('#button--6').click();
    cy.get('#button--0').click();
    cy.get('#calculator__input').should('have.text', '60');

    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '60');
  });

  it('adds values', () => {
    cy.get('#button--3').click();
    cy.get('#button--8').click();
    cy.get('#button--5').click();
    cy.get('#button--1').click();
    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '3851');

    cy.get('#button--2').click();
    cy.get('#button--7').click();
    cy.get('#calculator__input').should('have.text', '27');

    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '3878');
  });

  it('subtracts values', () => {
    cy.get('#button--2').click();
    cy.get('#button--7').click();
    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '27');

    cy.get('#button--3').click();
    cy.get('#button--8').click();
    cy.get('#button--4').click();
    cy.get('#calculator__input').should('have.text', '384');

    cy.get('#button--subtract').click();
    cy.get('#calculator__result').should('have.text', '-357');
  });

  it('multiplies values', () => {
    cy.get('#button--1').click();
    cy.get('#button--4').click();
    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '14');

    cy.get('#button--9').click();
    cy.get('#calculator__input').should('have.text', '9');

    cy.get('#button--multiply').click();
    cy.get('#calculator__result').should('have.text', '126');
  });

  it('divides values', () => {
    cy.get('#button--2').click();
    cy.get('#button--0').click();
    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '20');

    cy.get('#button--5').click();
    cy.get('#calculator__input').should('have.text', '5');

    cy.get('#button--divide').click();
    cy.get('#calculator__result').should('have.text', '4');
  });

  it('clears values', () => {
    cy.get('#button--3').click();
    cy.get('#button--4').click();
    cy.get('#button--6').click();
    cy.get('#button--add').click();
    cy.get('#calculator__result').should('have.text', '346');

    cy.get('#button--9').click();
    cy.get('#button--3').click();
    cy.get('#button--7').click();
    cy.get('#calculator__input').should('have.text', '937');

    cy.get('#button--clear').click();
    cy.get('#calculator__result').should('have.text', '0');
    cy.get('#calculator__input').should('have.text', '0');
  });
});
