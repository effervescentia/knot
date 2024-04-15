use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::CommonVisitor};

pub const fn analyze(
    x: &ast::TypeDeclaration<
        <Visitor as CommonVisitor>::Binding,
        <Visitor as CommonVisitor>::TypeExpression,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::TypeDeclaration::TypeAlias { .. } => None,

        ast::TypeDeclaration::View { .. } => None,
    }
}
