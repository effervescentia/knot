use crate::{error::Error, Context, Result};
use lang::{
    ast,
    walk::{Visit, Walk},
    NodeId,
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

impl<Meta> Visit<(lang::Range, Meta)> for Visitor {
    // type Binding = ast::typed::Binding;
    type Binding = ();
    // type Expression = ast::typed::Expression;
    type Expression = ();
    // type Statement = ast::typed::Statement;
    type Statement = ();
    // type Component = ast::typed::Component;
    type Component = ();
    // type TypeExpression = ast::typed::TypeExpression;
    type TypeExpression = ();
    // type Parameter = ast::typed::Parameter;
    type Parameter = ();
    // type Declaration = ast::typed::Declaration;
    type Declaration = ();
    // type Import = ast::typed::Import;
    type Import = ();
    // type Module = ast::typed::Module;
    type Module = ();

    fn binding(self, x: ast::Binding, r: lang::Range) -> (Self::Binding, Self) {
        ((), self)
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        ctx: (lang::Range, Meta),
    ) -> (Self::Expression, Self) {
        match x {
            ast::Expression::BinaryOperation(ast::BinaryOperator::And, lhs, rhs) => (),

            _ => (),
        }

        ((), self)
    }

    fn statement(
        self,
        x: ast::Statement<Self::Expression>,
        ctx: (lang::Range, Meta),
    ) -> (Self::Statement, Self) {
        ((), self)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        ctx: (lang::Range, Meta),
    ) -> (Self::Component, Self) {
        ((), self)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        ctx: (lang::Range, Meta),
    ) -> (Self::TypeExpression, Self) {
        ((), self)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        ctx: (lang::Range, Meta),
    ) -> (Self::Parameter, Self) {
        ((), self)
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
        ctx: (lang::Range, Meta),
    ) -> (Self::Declaration, Self) {
        ((), self)
    }

    fn import(self, x: ast::Import, ctx: (lang::Range, Meta)) -> (Self::Import, Self) {
        ((), self)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        ctx: (lang::Range, Meta),
    ) -> (Self::Module, Self) {
        ((), self)
    }
}
