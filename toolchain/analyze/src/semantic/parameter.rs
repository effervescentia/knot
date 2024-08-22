use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    types::ToShape,
    walk::{CommonVisitor, ProgramVisitor},
    Identify, TypeOf,
};

pub fn analyze(
    ast::Parameter {
        value_type,
        default_value,
        ..
    }: &ast::Parameter<
        <Visitor as CommonVisitor>::Binding,
        <Visitor as ProgramVisitor>::Expression,
        <Visitor as CommonVisitor>::TypeExpression,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match (value_type.as_ref(), default_value.as_ref()) {
        (Some(typedef), Some(default)) => {
            let typedef_shape = typedef.type_of().to_shape();
            let default_shape = default.type_of().to_shape();

            (typedef_shape != default_shape).then_some(vec![Error::DefaultValueRejected(
                (*typedef.id(), typedef_shape),
                (*default.id(), default_shape),
            )])
        }

        _ => None,
    }
}
