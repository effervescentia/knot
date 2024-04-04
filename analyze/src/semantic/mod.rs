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
struct Visitor(Vec<(NodeId, Error)>);

impl Visitor {
    fn node<T, M, R, F>(self, x: T, (r, t): (Range, M), f: F) -> (R, Self)
    where
        F: Fn(Node<T, M>) -> R,
    {
        (f(Node(x, r, t)), self)
    }
}

impl Visit<(lang::Range, ast::typed::Type)> for Visitor {
    type Binding = ast::typed::Binding;
    type Expression = ast::typed::Expression;
    type Statement = ast::typed::Statement;
    type Component = ast::typed::Component;
    type TypeExpression = ast::typed::TypeExpression;
    type Parameter = ast::typed::Parameter;
    type Declaration = ast::typed::Declaration;
    type Import = ast::typed::Import;
    type Module = ast::typed::Module;

    fn binding(self, x: ast::Binding, r: lang::Range) -> (Self::Binding, Self) {
        (ast::typed::Binding(Node::raw(x, r)), self)
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::Expression, Self) {
        match &x {
            ast::Expression::BinaryOperation(ast::BinaryOperator::And, lhs, rhs) => (),

            _ => (),
        }

        self.node(x, ctx, ast::meta::Expression)
    }

    fn statement(
        self,
        x: ast::Statement<Self::Expression>,
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::Statement, Self) {
        self.node(x, ctx, ast::meta::Statement)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::Component, Self) {
        self.node(x, ctx, ast::meta::Component)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::TypeExpression, Self) {
        self.node(x, ctx, ast::meta::TypeExpression)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        ctx: (lang::Range, ast::typed::Type),
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
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::Declaration, Self) {
        self.node(x, ctx, ast::meta::Declaration)
    }

    fn import(self, x: ast::Import, ctx: (lang::Range, ast::typed::Type)) -> (Self::Import, Self) {
        self.node(x, ctx, ast::meta::Import)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        ctx: (lang::Range, ast::typed::Type),
    ) -> (Self::Module, Self) {
        self.node(x, ctx, ast::meta::Module)
    }
}
