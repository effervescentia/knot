use super::{analyze::Analyzed, write::Output};
use crate::engine2::{pipeline::Transform, state::State};
use std::path::PathBuf;

pub struct Generated(Vec<(PathBuf, String)>);

impl Output for Generated {
    type Data = String;

    fn output(self) -> Vec<(PathBuf, Self::Data)> {
        self.0
    }
}

pub struct Generate<Tx, Gen>(Tx, Gen);

impl<Tx, Gen> Generate<Tx, Gen> {
    pub const fn bind(generator: Gen) -> impl FnOnce(Tx) -> Self {
        |tx| Self(tx, generator)
    }
}

impl<'a, Tx, Gen, Log> Transform for Generate<Tx, Gen>
where
    Tx: Transform<Out = (State<'a, Log>, Analyzed)>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Generated);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);

        (state, Generated(vec![]))
    }
}
