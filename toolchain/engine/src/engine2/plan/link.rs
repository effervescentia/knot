use super::parse::Parsed;
use crate::engine2::{pipeline::Transform, state::State};
use lang::ModuleId;
use std::collections::HashSet;

pub struct Linked(pub HashSet<ModuleId>);

impl From<Linked> for Parsed {
    fn from(val: Linked) -> Self {
        Self(val.0)
    }
}

pub struct Link<Tx>(Tx);

impl<Tx> Link<Tx> {
    pub const fn new(tx: Tx) -> Self {
        Self(tx)
    }
}

impl<'a, Tx, Res, Log> Transform for Link<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Res)>,
    Res: Into<Parsed>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Linked);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, result) = self.0.apply(input);
        let Parsed(ids) = result.into();

        // let linked = state.link_modules(context)?;

        // Validator(context).validate(&state, &linked)?;

        // Ok(state::Linked::new(state, linked));

        (state, Linked(ids))
    }
}
