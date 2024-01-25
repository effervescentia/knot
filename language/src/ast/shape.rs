use super::walk::{self, Walk};
use crate::Range;
use std::fmt::{Display, Formatter};

pub struct Expression(pub super::Expression<Expression, Statement, Component>);

impl Display for Expression {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Statement(pub super::Statement<Expression>);

impl Display for Statement {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Component(pub super::Component<Component, Expression>);

impl Display for Component {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct TypeExpression(pub super::TypeExpression<TypeExpression>);

impl Display for TypeExpression {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Parameter(pub super::Parameter<String, Expression, TypeExpression>);

impl Display for Parameter {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Declaration(
    pub super::Declaration<String, Expression, TypeExpression, Parameter, Module>,
);

impl Display for Declaration {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Import(pub super::Import);

impl Display for Import {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Module(pub super::Module<Import, Declaration>);

impl Display for Module {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Visitor;

impl walk::Visit for Visitor {
    type Binding = String;
    type Expression = Expression;
    type Statement = Statement;
    type Component = Component;
    type TypeExpression = TypeExpression;
    type Parameter = Parameter;
    type Declaration = Declaration;
    type Import = Import;
    type Module = Module;

    fn binding(self, x: super::Binding, _: Range) -> (Self::Binding, Self) {
        (x.0, self)
    }

    fn expression(
        self,
        x: super::Expression<Self::Expression, Self::Statement, Self::Component>,
        _: Range,
    ) -> (Self::Expression, Self) {
        (Expression(x), self)
    }

    fn statement(self, x: super::Statement<Self::Expression>, _: Range) -> (Self::Statement, Self) {
        (Statement(x), self)
    }

    fn component(
        self,
        x: super::Component<Self::Component, Self::Expression>,
        _: Range,
    ) -> (Self::Component, Self) {
        (Component(x), self)
    }

    fn type_expression(
        self,
        x: super::TypeExpression<Self::TypeExpression>,
        _: Range,
    ) -> (Self::TypeExpression, Self) {
        (TypeExpression(x), self)
    }

    fn parameter(
        self,
        x: super::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        _: Range,
    ) -> (Self::Parameter, Self) {
        (Parameter(x), self)
    }

    fn declaration(
        self,
        x: super::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        _: Range,
    ) -> (Self::Declaration, Self) {
        (Declaration(x), self)
    }

    fn import(self, x: super::Import, _: Range) -> (Self::Import, Self) {
        (Import(x), self)
    }

    fn module(
        self,
        x: super::Module<Self::Import, Self::Declaration>,
        _: Range,
    ) -> (Self::Module, Self) {
        (Module(x), self)
    }
}

impl<Import, Declaration> walk::Span<super::Module<Import, Declaration>>
where
    Import: walk::Walk<Visitor, Output = <Visitor as walk::Visit>::Import>,
    Declaration: walk::Walk<Visitor, Output = <Visitor as walk::Visit>::Declaration>,
{
    pub fn to_shape(self) -> Module {
        self.walk(Visitor).0
    }
}
