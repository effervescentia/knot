use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, TypingsVisitor},
};

pub const fn analyze(
    x: &ast::TypeDeclaration<
        <Visitor as CommonVisitor>::Binding,
        <Visitor as CommonVisitor>::TypeExpression,
        <Visitor as TypingsVisitor>::TypeModule,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::TypeDeclaration::TypeAlias { .. } => None,

        ast::TypeDeclaration::Enumerated { .. } => None,

        ast::TypeDeclaration::View { .. } => None,

        ast::TypeDeclaration::Function { .. } => None,

        ast::TypeDeclaration::Module { .. } => None,
    }
}
