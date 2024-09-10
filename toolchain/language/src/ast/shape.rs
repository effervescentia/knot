use crate::{
    walk::{CommonVisitor, ProgramVisitor, TypingsVisitor},
    Range,
};
use std::{
    fmt::{Display, Formatter},
    marker::PhantomData,
};

use super::IsEmpty;

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

pub struct Attribute(pub super::Attribute<Expression>);

impl Display for Attribute {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Component(pub super::Component<Component, Expression, Attribute>);

impl Display for Component {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct TypeExpression(pub super::TypeExpression<String, TypeExpression>);

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

pub struct Program(pub Module);

impl Display for Program {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct TypeDeclaration(pub super::TypeDeclaration<String, TypeExpression, TypeModule>);

impl Display for TypeDeclaration {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct TypeModule(pub super::TypeModule<Import, TypeDeclaration>);

impl IsEmpty for TypeModule {
    fn is_empty(&self) -> bool {
        self.0.is_empty()
    }
}

impl Display for TypeModule {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Typings(pub TypeModule);

impl Display for Typings {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

pub struct Visitor<Context>(PhantomData<Context>);

impl<Context> Default for Visitor<Context> {
    fn default() -> Self {
        Self(PhantomData)
    }
}

impl<Context> CommonVisitor for Visitor<Context> {
    type Context = Context;
    type Binding = String;
    type TypeExpression = TypeExpression;
    type Import = Import;

    fn binding(self, x: super::Binding, _: Range) -> (Self::Binding, Self) {
        (x.0, self)
    }

    fn type_expression(
        self,
        x: super::TypeExpression<Self::Binding, Self::TypeExpression>,
        _: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        (TypeExpression(x), self)
    }

    fn import(self, x: super::Import, _: Self::Context) -> (Self::Import, Self) {
        (Import(x), self)
    }
}

impl<Context> ProgramVisitor for Visitor<Context> {
    type Expression = Expression;
    type Statement = Statement;
    type Attribute = Attribute;
    type Component = Component;
    type Parameter = Parameter;
    type Declaration = Declaration;
    type Module = Module;

    fn expression(
        self,
        x: super::Expression<Self::Expression, Self::Statement, Self::Component>,
        _: Self::Context,
    ) -> (Self::Expression, Self) {
        (Expression(x), self)
    }

    fn statement(
        self,
        x: super::Statement<Self::Expression>,
        _: Self::Context,
    ) -> (Self::Statement, Self) {
        (Statement(x), self)
    }

    fn attribute(
        self,
        x: super::Attribute<Self::Expression>,
        _: Self::Context,
    ) -> (Self::Attribute, Self) {
        (Attribute(x), self)
    }

    fn component(
        self,
        x: super::Component<Self::Component, Self::Expression, Self::Attribute>,
        _: Self::Context,
    ) -> (Self::Component, Self) {
        (Component(x), self)
    }

    fn parameter(
        self,
        x: super::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        _: Self::Context,
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
        _: Self::Context,
    ) -> (Self::Declaration, Self) {
        (Declaration(x), self)
    }

    fn module(
        self,
        x: super::Module<Self::Import, Self::Declaration>,
        _: Self::Context,
    ) -> (Self::Module, Self) {
        (Module(x), self)
    }
}

impl<Context> TypingsVisitor for Visitor<Context> {
    type TypeDeclaration = TypeDeclaration;
    type TypeModule = TypeModule;

    fn type_declaration(
        self,
        x: super::TypeDeclaration<Self::Binding, Self::TypeExpression, Self::TypeModule>,
        _: Self::Context,
    ) -> (Self::TypeDeclaration, Self) {
        (TypeDeclaration(x), self)
    }

    fn type_module(
        self,
        x: super::TypeModule<Self::Import, Self::TypeDeclaration>,
        _: Self::Context,
    ) -> (Self::TypeModule, Self) {
        (TypeModule(x), self)
    }
}
