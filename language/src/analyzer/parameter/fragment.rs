use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        declaration::parameter::{self, Parameter},
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for parameter::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Parameter(Parameter::new(
            self.name.clone(),
            self.value_type.as_ref().map(|x| *x.0.id()),
            self.default_value.as_ref().map(|x| *x.0.id()),
        ))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        parser::{
            declaration::parameter::Parameter,
            expression::{primitive::Primitive, Expression},
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        assert_eq!(
            Parameter::new(
                String::from("foo"),
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                Some(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                ))
            )
            .to_fragment(),
            Fragment::Parameter(Parameter::new(String::from("foo"), Some(0), Some(1)))
        );
    }
}
