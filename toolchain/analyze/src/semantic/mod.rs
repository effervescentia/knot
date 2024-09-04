mod attribute;
mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_declaration;
mod type_expression;
mod type_module;

use crate::{error::Error, Context, Result, TypeMap};
use lang::{
    ast,
    walk::{CommonVisitor, ProgramVisitor, TypingsVisitor, Walk},
    Identify, Node, NodeId, Range,
};

pub fn analyze<'a, Typed>(ctx: &'a Context<'a>, typed: Typed, types: TypeMap) -> Result<Typed>
where
    Typed: Clone + Walk<Visitor<'a>>,
{
    let visitor = Visitor::new(ctx, types);

    let (_, visitor) = typed.clone().walk(visitor);

    if visitor.errors.is_empty() {
        Ok(typed)
    } else {
        Err(visitor.errors)
    }
}

pub struct Visitor<'a> {
    errors: Vec<(NodeId, Error)>,
    pub context: &'a Context<'a>,
    pub types: TypeMap,
}

impl<'a> Visitor<'a> {
    fn new(context: &'a Context<'a>, types: TypeMap) -> Self {
        Self {
            errors: Default::default(),
            context,
            types,
        }
    }

    fn node<T, M, R, F>(self, x: T, (r, m): (Range, M), f: F) -> (R, Self)
    where
        F: Fn(Node<T, M>) -> R,
    {
        (f(Node(x, r, m)), self)
    }

    fn report<T, F>(&mut self, x: &T, ctx: &<Self as CommonVisitor>::Context, analyzer: F)
    where
        F: Fn(&T, &<Self as CommonVisitor>::Context, &Self) -> Option<Vec<Error>>,
    {
        if let Some(errors) = analyzer(x, ctx, self) {
            self.errors
                .extend(errors.into_iter().map(|err| (ctx.id().1, err)));
        }
    }
}

impl<'a> CommonVisitor for Visitor<'a> {
    type Context = (Range, ast::typed::Meta);
    type Binding = ast::typed::Binding;
    type TypeExpression = ast::typed::TypeExpression;

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self) {
        (ast::typed::Binding(Node::raw(x, r)), self)
    }

    fn type_expression(
        mut self,
        x: ast::TypeExpression<Self::Binding, Self::TypeExpression>,
        ctx: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.report(&x, &ctx, type_expression::analyze);

        self.node(x, ctx, ast::meta::TypeExpression)
    }
}

impl<'a> ProgramVisitor for Visitor<'a> {
    type Expression = ast::typed::Expression;
    type Statement = ast::typed::Statement;
    type Attribute = ast::typed::Attribute;
    type Component = ast::typed::Component;
    type Parameter = ast::typed::Parameter;
    type Declaration = ast::typed::Declaration;
    type Import = ast::typed::Import;
    type Module = ast::typed::Module;

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

    fn attribute(
        mut self,
        x: ast::Attribute<Self::Expression>,
        ctx: Self::Context,
    ) -> (Self::Attribute, Self) {
        self.report(&x, &ctx, attribute::analyze);

        self.node(x, ctx, ast::meta::Attribute)
    }

    fn component(
        mut self,
        x: ast::Component<Self::Component, Self::Expression, Self::Attribute>,
        ctx: Self::Context,
    ) -> (Self::Component, Self) {
        self.report(&x, &ctx, component::analyze);

        self.node(x, ctx, ast::meta::Component)
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

impl<'a> TypingsVisitor for Visitor<'a> {
    type TypeDeclaration = ast::typed::TypeDeclaration;
    type TypeModule = ast::typed::TypeModule;

    fn type_declaration(
        mut self,
        x: ast::TypeDeclaration<Self::Binding, Self::TypeExpression, Self::TypeModule>,
        ctx: Self::Context,
    ) -> (Self::TypeDeclaration, Self) {
        self.report(&x, &ctx, type_declaration::analyze);

        self.node(x, ctx, ast::meta::TypeDeclaration)
    }

    fn type_module(
        mut self,
        x: ast::TypeModule<Self::TypeDeclaration>,
        ctx: Self::Context,
    ) -> (Self::TypeModule, Self) {
        self.report(&x, &ctx, type_module::analyze);

        self.node(x, ctx, ast::meta::TypeModule)
    }
}
