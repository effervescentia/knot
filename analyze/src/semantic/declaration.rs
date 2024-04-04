use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

#[allow(clippy::type_complexity)]
pub const fn analyze(
    x: &ast::Declaration<
        <Visitor as Visit>::Binding,
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::TypeExpression,
        <Visitor as Visit>::Parameter,
        <Visitor as Visit>::Module,
    >,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    match x {
        ast::Declaration::TypeAlias { .. } => None,

        ast::Declaration::Constant { .. } => None,

        ast::Declaration::Enumerated { .. } => None,

        ast::Declaration::Function { .. } => None,

        ast::Declaration::View { .. } => None,

        ast::Declaration::Module { .. } => None,
    }
}
