use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(
    _: &ast::Parameter<
        <Visitor as Visit>::Binding,
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::TypeExpression,
    >,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    None
}
