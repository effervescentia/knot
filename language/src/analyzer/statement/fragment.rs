use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        position::Decrement,
        statement::{self, Statement},
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for statement::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Statement(match self {
            Statement::Effect(x) => Statement::Effect(*x.node().id()),

            Statement::Variable(name, x) => Statement::Variable(name.clone(), *x.node().id()),
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        parser::{
            expression::{primitive::Primitive, Expression},
            statement::Statement,
        },
        test::fixture as f,
    };

    #[test]
    fn effect() {
        assert_eq!(
            Statement::Effect(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
            .to_fragment(),
            Fragment::Statement(Statement::Effect(0))
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
