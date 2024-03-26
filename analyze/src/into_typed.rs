use crate::{
    ast::{self, walk::Walk},
    infer, typed,
};
use kore::{invariant, Incrementor};
use lang::{Node, NodeId, Range};
use std::{cell::OnceCell, ops::Deref};

pub trait IntoTyped: Sized {
    fn into_typed(self, strong: Visitor) -> typed::Program;
}

impl<Context> IntoTyped for ast::meta::Program<Context> {
    fn into_typed(self, strong: super::Visitor) -> typed::Program {
        ast::meta::Program(self.0.walk(strong).0)
    }
}

pub struct Visitor {
    node_id: Incrementor,
    strong: infer::strong::Output,
}

impl Visitor {
    pub fn new(strong: infer::strong::Output) -> Self {
        Self {
            node_id: Default::default(),
            strong,
        }
    }

    fn next_type(&mut self) -> typed::Type {
        let id = NodeId(self.node_id.increment());

        self.strong
            .types
            .get(&id)
            .and_then(OnceCell::get)
            .unwrap_or_else(|| invariant!("type not found"))
            .deref()
            .clone()
    }

    fn typed<T, R, F>(mut self, x: T, r: Range, f: F) -> (R, Self)
    where
        F: Fn(Node<T, typed::Type>) -> R,
    {
        (f(Node(x, r, self.next_type())), self)
    }
}

impl ast::walk::Visit for Visitor {
    type Binding = typed::Binding;
    type Expression = typed::Expression;
    type Statement = typed::Statement;
    type Component = typed::Component;
    type TypeExpression = typed::TypeExpression;
    type Parameter = typed::Parameter;
    type Declaration = typed::Declaration;
    type Import = typed::Import;
    type Module = typed::Module;

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self) {
        (typed::Binding(Node::raw(x, r)), self)
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: Range,
    ) -> (Self::Expression, Self) {
        self.typed(x, r, ast::meta::Expression)
    }

    fn statement(self, x: ast::Statement<Self::Expression>, r: Range) -> (Self::Statement, Self) {
        self.typed(x, r, ast::meta::Statement)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        r: Range,
    ) -> (Self::Component, Self) {
        self.typed(x, r, ast::meta::Component)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        r: Range,
    ) -> (Self::TypeExpression, Self) {
        self.typed(x, r, ast::meta::TypeExpression)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Parameter, Self) {
        self.typed(x, r, ast::meta::Parameter)
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
        r: Range,
    ) -> (Self::Declaration, Self) {
        self.typed(x, r, ast::meta::Declaration)
    }

    fn import(self, x: ast::Import, r: Range) -> (Self::Import, Self) {
        self.typed(x, r, ast::meta::Import)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        r: Range,
    ) -> (Self::Module, Self) {
        self.typed(x, r, ast::meta::Module)
    }
}
