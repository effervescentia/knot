use super::parse::Parsed;
use crate::engine2::{pipeline::Transform, state::State};

pub struct Linked;

pub struct Link<Tx>(pub Tx);

impl<'a, Tx> Transform for Link<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    type In = Tx::In;
    type Out = (State<'a>, Linked);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);

        // let linked = state.link_modules(context)?;

        // Validator(context).validate(&state, &linked)?;

        // Ok(state::Linked::new(state, linked));

        (state, Linked)
    }
}
