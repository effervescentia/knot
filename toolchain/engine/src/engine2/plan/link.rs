use super::parse::Parsed;
use crate::engine2::{pipeline::Transform, state::State};

pub struct Linked;

pub struct Link<Tx>(Tx);

impl<Tx> Link<Tx> {
    pub const fn new(tx: Tx) -> Self {
        Self(tx)
    }
}

impl<'a, Tx, Log> Transform for Link<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Parsed)>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Linked);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);

        // let linked = state.link_modules(context)?;

        // Validator(context).validate(&state, &linked)?;

        // Ok(state::Linked::new(state, linked));

        (state, Linked)
    }
}
