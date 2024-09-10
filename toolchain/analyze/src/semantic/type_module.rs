use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, TypingsVisitor},
};

pub const fn analyze(
    _: &ast::TypeModule<
        <Visitor as CommonVisitor>::Import,
        <Visitor as TypingsVisitor>::TypeDeclaration,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    None
}
