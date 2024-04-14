use super::{component, Visitor};
use crate::error::Error;
use lang::{ast, walk::Visit, Identify, TypeOf};

#[allow(clippy::type_complexity)]
pub fn analyze(
    x: &ast::Declaration<
        <Visitor as Visit>::Binding,
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::TypeExpression,
        <Visitor as Visit>::Parameter,
        <Visitor as Visit>::Module,
    >,
    _: &<Visitor as Visit>::Context,
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
