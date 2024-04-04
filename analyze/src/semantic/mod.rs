mod component;
mod expression;
mod statement;

use crate::{error::Error, Context, Result};
use lang::{
    ast,
    walk::{Visit, Walk},
    Node, NodeId, Range,
};

pub fn analyze(ctx: &Context, typed: ast::typed::Program) -> Result<ast::typed::Program> {
    let visitor = Visitor::default();

    let (_, visitor) = typed.0.clone().walk(visitor);

    if visitor.0.is_empty() {
        Ok(typed)
    } else {
        Err(visitor.0)
    }
}

#[derive(Default)]
pub struct Visitor(Vec<(NodeId, Error)>);

impl Visitor {
    fn node<T, M, R, F>(self, x: T, (r, m): (Range, M), f: F) -> (R, Self)
    where
        F: Fn(Node<T, M>) -> R,
    {
        (f(Node(x, r, m)), self)
    }

    pub fn report(&mut self, ctx: &<Self as Visit>::Context, errors: Option<Vec<Error>>) {
        let (_, (canonical_id, _)) = ctx;

        if let Some(errors) = errors {
            self.0
                .extend(errors.into_iter().map(|err| (canonical_id.1, err)));
        }
    }
}

impl Visit for Visitor {
    type Context = (Range, ast::typed::Meta);
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
        mut self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        ctx: Self::Context,
    ) -> (Self::Expression, Self) {
        self.report(&ctx, expression::analyze(&x));

        self.node(x, ctx, ast::meta::Expression)
    }

    fn statement(
        mut self,
        x: ast::Statement<Self::Expression>,
        ctx: Self::Context,
    ) -> (Self::Statement, Self) {
        self.report(&ctx, statement::analyze(&x));

        self.node(x, ctx, ast::meta::Statement)
    }

    fn component(
        mut self,
        x: ast::Component<Self::Component, Self::Expression>,
        ctx: Self::Context,
    ) -> (Self::Component, Self) {
        self.report(&ctx, component::analyze(&x));

        self.node(x, ctx, ast::meta::Component)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        ctx: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.node(x, ctx, ast::meta::TypeExpression)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        ctx: Self::Context,
    ) -> (Self::Parameter, Self) {
        self.node(x, ctx, ast::meta::Parameter)
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
        ctx: Self::Context,
    ) -> (Self::Declaration, Self) {
        self.node(x, ctx, ast::meta::Declaration)
    }

    fn import(self, x: ast::Import, ctx: Self::Context) -> (Self::Import, Self) {
        self.node(x, ctx, ast::meta::Import)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        ctx: Self::Context,
    ) -> (Self::Module, Self) {
        self.node(x, ctx, ast::meta::Module)
    }
}
