use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    ast::parameter,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for parameter::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Parameter(self.map(&|x| *x.node().id(), &|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        ast::{
            expression::{Expression, Primitive},
            parameter::Parameter,
            type_expression::TypeExpression,
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
