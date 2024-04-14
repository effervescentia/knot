use super::Visitor;
use crate::error::Error;
use lang::{ast, types::ToShape, walk::Visit, Identify, TypeOf};

pub fn analyze(
    ast::Parameter {
        value_type,
        default_value,
        ..
    }: &ast::Parameter<
        <Visitor as Visit>::Binding,
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::TypeExpression,
    >,
    _: &<Visitor as Visit>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match (value_type.as_ref(), default_value.as_ref()) {
        (Some(typdef), Some(default)) => (typdef.type_of().to_shape()
            != default.type_of().to_shape())
        .then_some(vec![Error::DefaultValueRejected(*default.id())]),

        _ => None,
    }
}
