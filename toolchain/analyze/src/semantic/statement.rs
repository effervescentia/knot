use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, ProgramVisitor},
};

pub const fn analyze(
    x: &ast::Statement<<Visitor as ProgramVisitor>::Expression>,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Statement::Expression(_) => None,

        ast::Statement::Variable(..) => None,
    }
}
