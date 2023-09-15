use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    parser::{
        position::Decrement,
        statement::{self, Statement},
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<statement::NodeValue<T, NodeContext>> for statement::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> statement::NodeValue<T, NodeContext> {
        match self {
            Statement::Effect(x) => Statement::Effect(x.register(ctx)),

            Statement::Variable(name, x) => Statement::Variable(name, x.register(ctx)),
        }
    }
}
