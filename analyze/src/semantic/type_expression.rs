use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(
    x: &ast::TypeExpression<<Visitor as Visit>::TypeExpression>,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    match x {
        ast::TypeExpression::Primitive(_) => None,

        ast::TypeExpression::Identifier(_) => None,

        ast::TypeExpression::Group(_) => None,

        ast::TypeExpression::PropertyAccess(..) => None,

        ast::TypeExpression::Function(..) => None,
    }
}
