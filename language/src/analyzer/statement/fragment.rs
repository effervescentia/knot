use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        expression::statement::{self, Statement},
        position::Decrement,
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
