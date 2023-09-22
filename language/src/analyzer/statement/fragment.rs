use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    ast::statement,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for statement::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Statement(self.map(&|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
        },
        test::fixture as f,
    };

    #[test]
    fn expression() {
        assert_eq!(
            Statement::Expression(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
            .to_fragment(),
            Fragment::Statement(Statement::Expression(0))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(
                String::from("foo"),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )
            )
            .to_fragment(),
            Fragment::Statement(Statement::Variable(String::from("foo"), 0))
        );
    }
}
