use super::parse::Parsed;
use crate::{
    engine2::{pipeline::Transform, state::State},
    validate::Validator2,
};
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
        let (mut state, result) = self.0.apply(input);
        let Parsed(ids) = result.into();

        for id in &ids {
            if let Some(module) = state.get_module(id) {
                let dependencies = module.raw.get_dependencies(&module.path);

                for dependency in &dependencies {
                    if let Some(dependency_id) = state.registry.get_id(dependency) {
                        state.add_dependency(id, &dependency_id);
                    } else {
                        panic!("replace this with an actual error");
                    }
                }
            }
        }

        Validator2.validate(&state);

        (state, Linked(ids))
    }
}
