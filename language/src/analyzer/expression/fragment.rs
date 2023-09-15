use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        expression::{self, Expression},
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for expression::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Expression(match self {
            Expression::Primitive(x) => Expression::Primitive(x.clone()),

            Expression::Identifier(x) => Expression::Identifier(x.clone()),

            Expression::Group(x) => Expression::Group(Box::new(*x.node().id())),

            Expression::Closure(xs) => {
                Expression::Closure(xs.into_iter().map(|x| *x.node().id()).collect::<Vec<_>>())
            }

            Expression::UnaryOperation(op, x) => {
                Expression::UnaryOperation(op.clone(), Box::new(*x.node().id()))
            }

            Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                op.clone(),
                Box::new(*lhs.node().id()),
                Box::new(*rhs.node().id()),
            ),

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(*lhs.node().id()), rhs.clone())
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(*x.node().id()),
                args.into_iter().map(|x| *x.node().id()).collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key.clone(), *value.node().id()))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(*x.node().id())),
        })
    }
}
