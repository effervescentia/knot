use super::{link::Linked, Parsed};
use crate::engine2::{pipeline::Transform, state::State};
use lang::ModuleId;
use std::collections::HashSet;

pub struct Analyzed(pub HashSet<ModuleId>);

impl From<Analyzed> for Parsed {
    fn from(val: Analyzed) -> Self {
        Self(val.0)
    }
}

impl From<Analyzed> for Linked {
    fn from(val: Analyzed) -> Self {
        Self(val.0)
    }
}

pub struct Analyze<Tx>(Tx);

impl<Tx> Analyze<Tx> {
    pub const fn new(tx: Tx) -> Self {
        Self(tx)
    }
}

impl<'a, Tx, Res, Log> Transform for Analyze<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Res)>,
    Res: Into<Linked>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Analyzed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, result) = self.0.apply(input);
        let Linked(ids) = result.into();

        let mut modules = analyze::ModuleMap::default();

        // for id in state.graph().iter() {
        //     let analyze_context = analyze::Context {
        //         id: module.id,
        //         namespace: &namespace,
        //         modules: &modules,
        //         ambient: state.ambient(),
        //     };

        //     // TODO: see if it's possible to fail after all modules are processed instead of immediately
        //     let (typed, types) = module
        //         .ast
        //         .analyze(&analyze_context)
        //         .map_err(|errs| context.fail(Self::bind_errors(&analyze_context, errs)))?;

        //     modules.keys.insert(namespace, module.id);
        //     modules
        //         .by_key
        //         .insert(module.id, (*typed.id(), typed.exports(), types));
        //     analyzed.insert(
        //         link.clone(),
        //         state::Module::new(module.id, module.text.clone(), typed),
        //     );
        // }

        // for id in state.iter_graph() {
        //     let (link, module) = Self::get_module(&state, &id);
        //     let namespace = link.clone().to_namespace();
        //     let analyze_context = analyze::Context {
        //         id: module.id,
        //         namespace: &namespace,
        //         modules: &modules,
        //         ambient: state.ambient(),
        //     };

        //     // TODO: see if it's possible to fail after all modules are processed instead of immediately
        //     let (typed, types) = module
        //         .ast
        //         .analyze(&analyze_context)
        //         .map_err(|errs| context.fail(Self::bind_errors(&analyze_context, errs)))?;

        //     modules.keys.insert(namespace, module.id);
        //     modules
        //         .by_key
        //         .insert(module.id, (*typed.id(), typed.exports(), types));
        //     analyzed.insert(
        //         link.clone(),
        //         state::Module::new(module.id, module.text.clone(), typed),
        //     );
        // }

        (state, Analyzed(ids))
    }
}
