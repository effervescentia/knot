use super::{component, Visitor};
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, ProgramVisitor},
    Identify, TypeOf,
};

#[allow(clippy::type_complexity)]
pub fn analyze(
    x: &ast::Declaration<
        <Visitor as CommonVisitor>::Binding,
        <Visitor as ProgramVisitor>::Expression,
        <Visitor as CommonVisitor>::TypeExpression,
        <Visitor as ProgramVisitor>::Parameter,
        <Visitor as ProgramVisitor>::Module,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Declaration::TypeAlias { .. } => None,

        ast::Declaration::Constant { .. } => None,

        ast::Declaration::Enumerated { .. } => None,

        ast::Declaration::Function { .. } => None,

        ast::Declaration::View { body, .. } => (!component::can_render(body.type_of()))
            .then_some(vec![Error::NotRenderable(*body.id())]),

        ast::Declaration::Module { .. } => None,
    }
}
