use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    parser::{
        declaration::parameter::{self, Parameter},
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<parameter::NodeValue<T, NodeContext>> for parameter::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> parameter::NodeValue<T, NodeContext> {
        Parameter {
            name: self.name,
            value_type: self.value_type.map(|x| x.register(ctx)),
            default_value: self.default_value.map(|x| x.register(ctx)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        parser::{
            declaration::parameter::Parameter,
            expression::{primitive::Primitive, Expression},
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            Parameter::new(
                String::from("foo"),
                Some(f::n::tx(TypeExpression::Nil)),
                Some(f::n::x(Expression::Primitive(Primitive::Nil)))
            )
            .identify(scope),
            Parameter::new(
                String::from("foo"),
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                Some(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                ))
            )
        );
    }
}
