use crate::{
    ast::{
        self,
        walk::{Visit, Walk},
    },
    Node, Range,
};

pub trait NilRange {
    fn nil_range(self) -> Self;
}

impl NilRange for ast::meta::Binding {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Expression {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Statement {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Component {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::TypeExpression {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Parameter {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Declaration {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Import {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

impl NilRange for ast::raw::Module {
    fn nil_range(self) -> Self {
        self.walk(Visitor).0
    }
}

struct Visitor;

impl<Context> Visit<Context> for Visitor {
    type Binding = ast::raw::Binding;
    type Expression = ast::raw::Expression;
    type Statement = ast::raw::Statement;
    type Component = ast::raw::Component;
    type TypeExpression = ast::raw::TypeExpression;
    type Parameter = ast::raw::Parameter;
    type Declaration = ast::raw::Declaration;
    type Import = ast::raw::Import;
    type Module = ast::raw::Module;

    fn binding(self, x: ast::Binding, _: Range) -> (Self::Binding, Self) {
        (ast::meta::Binding(Node::raw(x, Range::nil())), self)
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        _: Context,
    ) -> (Self::Expression, Self) {
        (ast::meta::Expression(Node::raw(x, Range::nil())), self)
    }

    fn statement(self, x: ast::Statement<Self::Expression>, _: Context) -> (Self::Statement, Self) {
        (ast::meta::Statement(Node::raw(x, Range::nil())), self)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        _: Context,
    ) -> (Self::Component, Self) {
        (ast::meta::Component(Node::raw(x, Range::nil())), self)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        _: Context,
    ) -> (Self::TypeExpression, Self) {
        (ast::meta::TypeExpression(Node::raw(x, Range::nil())), self)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        _: Context,
    ) -> (Self::Parameter, Self) {
        (ast::meta::Parameter(Node::raw(x, Range::nil())), self)
    }

    fn declaration(
        self,
        x: ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        _: Context,
    ) -> (Self::Declaration, Self) {
        (ast::meta::Declaration(Node::raw(x, Range::nil())), self)
    }

    fn import(self, x: ast::Import, _: Context) -> (Self::Import, Self) {
        (ast::meta::Import(Node::raw(x, Range::nil())), self)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        _: Context,
    ) -> (Self::Module, Self) {
        (ast::meta::Module(Node::raw(x, Range::nil())), self)
    }
}
