use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, ProgramVisitor},
};

pub const fn analyze(
    _: &ast::Module<<Visitor as CommonVisitor>::Import, <Visitor as ProgramVisitor>::Declaration>,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    None
}
