mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::{error::Error, Context, Result, TypeMap};
use lang::{
    ast,
    walk::{Visit, Walk},
    Identify, Node, NodeId, Range,
};

pub fn analyze(
    _: &Context,
    typed: ast::typed::Program,
    types: &TypeMap,
) -> Result<ast::typed::Program> {
    let visitor = Visitor::new(types);

    let (_, visitor) = typed.0.clone().walk(visitor);

    if visitor.errors.is_empty() {
        Ok(typed)
    } else {
        Err(visitor.errors)
    }
}

pub struct Visitor<'a> {
    errors: Vec<(NodeId, Error)>,
    types: &'a TypeMap,
}

impl<'a> Visitor<'a> {
    fn new(types: &'a TypeMap) -> Self {
        Self {
            types,
            errors: Default::default(),
        }
    }

    fn node<T, M, R, F>(self, x: T, (r, m): (Range, M), f: F) -> (R, Self)
    where
        F: Fn(Node<T, M>) -> R,
    {
        (f(Node(x, r, m)), self)
    }

    fn report<T, F>(&mut self, x: &T, ctx: &<Self as Visit>::Context, analyzer: F)
    where
        F: Fn(&T, &<Self as Visit>::Context) -> Option<Vec<Error>>,
    {
        if let Some(errors) = analyzer(x, ctx) {
            self.errors
                .extend(errors.into_iter().map(|err| (ctx.id().1, err)));
        }
    }
}

impl<'a> Visit for Visitor<'a> {
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
        self.report(&x, &ctx, expression::analyze);

        self.node(x, ctx, ast::meta::Expression)
    }

    fn statement(
        mut self,
        x: ast::Statement<Self::Expression>,
        ctx: Self::Context,
    ) -> (Self::Statement, Self) {
        self.report(&x, &ctx, statement::analyze);

        self.node(x, ctx, ast::meta::Statement)
    }

    fn component(
        mut self,
        x: ast::Component<Self::Component, Self::Expression>,
        ctx: Self::Context,
    ) -> (Self::Component, Self) {
        self.report(&x, &ctx, component::analyze);

        self.node(x, ctx, ast::meta::Component)
    }

    fn type_expression(
        mut self,
        x: ast::TypeExpression<Self::TypeExpression>,
        ctx: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.report(&x, &ctx, type_expression::analyze);

        self.node(x, ctx, ast::meta::TypeExpression)
    }

    fn parameter(
        mut self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        ctx: Self::Context,
    ) -> (Self::Parameter, Self) {
        self.report(&x, &ctx, parameter::analyze);

        self.node(x, ctx, ast::meta::Parameter)
    }

    fn declaration(
        mut self,
        x: ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        ctx: Self::Context,
    ) -> (Self::Declaration, Self) {
        self.report(&x, &ctx, declaration::analyze);

        self.node(x, ctx, ast::meta::Declaration)
    }

    fn import(mut self, x: ast::Import, ctx: Self::Context) -> (Self::Import, Self) {
        self.report(&x, &ctx, import::analyze);

        self.node(x, ctx, ast::meta::Import)
    }

    fn module(
        mut self,
        x: ast::Module<Self::Import, Self::Declaration>,
        ctx: Self::Context,
    ) -> (Self::Module, Self) {
        self.report(&x, &ctx, module::analyze);

        self.node(x, ctx, ast::meta::Module)
    }
}
