use crate::{
    ast::{self, walk::Walk},
    infer,
};
use kore::{invariant, Incrementor};
use lang::{Node, NodeId, Range};
use std::cell::OnceCell;

pub trait IntoTyped: Sized {
    fn into_typed(self, strong: Visitor) -> ast::typed::Program;
}

impl<Context> IntoTyped for ast::meta::Program<Context> {
    fn into_typed(self, strong: super::Visitor) -> ast::typed::Program {
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

    fn next_type(&mut self) -> ast::typed::Type {
        let id = NodeId(self.node_id.increment());

        self.strong
            .types
            .get(&id)
            .and_then(OnceCell::get)
            .unwrap_or_else(|| invariant!("type not found"))
            .1
            .clone()
    }

    fn typed<T, R, F>(mut self, x: T, r: Range, f: F) -> (R, Self)
    where
        F: Fn(Node<T, ast::typed::Type>) -> R,
    {
        (f(Node(x, r, self.next_type())), self)
    }
}

impl ast::walk::Visit for Visitor {
    type Binding = ast::typed::Binding;
    type Expression = ast::typed::Expression;
    type Statement = ast::typed::Statement;
    type Component = ast::typed::Component;
    type TypeExpression = ast::typed::TypeExpression;
    type Parameter = ast::typed::Parameter;
    type Declaration = ast::typed::Declaration;
    type Import = ast::typed::Import;
    type Module = ast::typed::Module;

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self) {
        (ast::typed::Binding(Node::raw(x, r)), self)
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
