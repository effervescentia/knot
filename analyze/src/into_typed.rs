use crate::{ast, infer, Context};
use kore::{invariant, Incrementor};
use lang::{
    walk::{Visit, Walk},
    CanonicalId, NamespaceId, Node, NodeId, Range,
};
use std::{cell::OnceCell, marker::PhantomData};

pub trait IntoTyped: Sized {
    fn into_typed(self, ctx: &Context, strong: &infer::strong::Output) -> ast::typed::Program;
}

impl<Meta> IntoTyped for ast::meta::Program<Meta> {
    fn into_typed(self, ctx: &Context, strong: &infer::strong::Output) -> ast::typed::Program {
        let visitor = Visitor::new(ctx.namespace_id, strong);
        ast::meta::Program(self.0.walk(visitor).0)
    }
}

struct Visitor<'a, Meta> {
    _meta: PhantomData<Meta>,
    node_id: Incrementor,
    namespace_id: NamespaceId,
    strong: &'a infer::strong::Output,
}

impl<'a, Meta> Visitor<'a, Meta> {
    const fn canonical(&self, node_id: NodeId) -> CanonicalId {
        CanonicalId(self.namespace_id, node_id)
    }

    pub fn new(namespace_id: NamespaceId, strong: &'a infer::strong::Output) -> Self {
        Self {
            _meta: PhantomData,
            node_id: Default::default(),
            namespace_id,
            strong,
        }
    }

    fn next_meta(&mut self) -> ast::typed::Meta {
        let node_id = NodeId(self.node_id.increment());
        let type_ = self
            .strong
            .types
            .get(&node_id)
            .and_then(OnceCell::get)
            .unwrap_or_else(|| invariant!("type not found"))
            .1
            .clone();

        (self.canonical(node_id), type_)
    }

    fn typed<T, R, F>(mut self, x: T, r: Range, f: F) -> (R, Self)
    where
        F: Fn(Node<T, ast::typed::Meta>) -> R,
    {
        (f(Node(x, r, self.next_meta())), self)
    }
}

impl<'a, Meta> Visit for Visitor<'a, Meta> {
    type Context = (Range, Meta);
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
        (r, _): Self::Context,
    ) -> (Self::Expression, Self) {
        self.typed(x, r, ast::meta::Expression)
    }

    fn statement(
        self,
        x: ast::Statement<Self::Expression>,
        (r, _): Self::Context,
    ) -> (Self::Statement, Self) {
        self.typed(x, r, ast::meta::Statement)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        (r, _): Self::Context,
    ) -> (Self::Component, Self) {
        self.typed(x, r, ast::meta::Component)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::Binding, Self::TypeExpression>,
        (r, _): Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.typed(x, r, ast::meta::TypeExpression)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        (r, _): Self::Context,
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
        (r, _): Self::Context,
    ) -> (Self::Declaration, Self) {
        self.typed(x, r, ast::meta::Declaration)
    }

    fn import(self, x: ast::Import, (r, _): Self::Context) -> (Self::Import, Self) {
        self.typed(x, r, ast::meta::Import)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        (r, _): Self::Context,
    ) -> (Self::Module, Self) {
        self.typed(x, r, ast::meta::Module)
    }
}
