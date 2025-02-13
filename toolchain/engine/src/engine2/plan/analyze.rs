use super::link::Linked;
use crate::engine2::{pipeline::Transform, state::State};

pub struct Analyzed;

pub struct Analyze<Tx>(Tx);

impl<Tx> Analyze<Tx> {
    pub const fn new(tx: Tx) -> Self {
        Self(tx)
    }
}

impl<'a, Tx, Log> Transform for Analyze<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Linked)>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Analyzed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Analyzed)
    }
}
