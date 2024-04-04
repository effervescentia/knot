use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(
    x: &ast::Expression<
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::Statement,
        <Visitor as Visit>::Component,
    >,
) -> Option<Vec<Error>> {
    match x {
        ast::Expression::Primitive(x) => (),

        ast::Expression::Identifier(x) => (),

        ast::Expression::Group(x) => (),

        ast::Expression::Closure(xs) => (),

        ast::Expression::UnaryOperation(op, x) => (),

        ast::Expression::BinaryOperation(op, lhs, rhs) => (),

        ast::Expression::PropertyAccess(x, property) => (),

        ast::Expression::FunctionCall(x, arguments) => (),

        ast::Expression::Style(xs) => (),

        ast::Expression::Component(x) => (),
    }

    None
}
