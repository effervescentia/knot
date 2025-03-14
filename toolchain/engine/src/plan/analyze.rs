use super::{Linked, Parsed};
use crate::State;
use kore::pipeline::Transform;
use lang::ModuleId;
use std::collections::{HashMap, HashSet};

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
    Tx: Transform<Context = State<'a, Log>, Out = Res>,
    Res: Into<Linked>,
    Log: 'a,
{
    type Context = Tx::Context;
    type In = Tx::In;
    type Out = Analyzed;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        let (mut state, result) = self.0.apply(input);
        let Linked(ids) = result.into();

        let mut analyzed = HashMap::new();
        let mut modules = analyze::ModuleMap::default();

        // iterate through the modules in import order
        for module in state.modules.iter() {
            // skip the modules that aren't the focus of this operation
            if !ids.contains(&module.id) {
                continue;
            }

            // TODO: this should be derived from the state
            let ambient = HashMap::default();
            let context = analyze::Context {
                id: module.id,
                namespace: &module.namespace,
                modules: &modules,
                ambient: &ambient,
            };

            // TODO: it should be possible to fail after all modules are processed instead of immediately
            let (typed, types) = module.raw.analyze(&context).unwrap();

            modules.keys.insert(module.namespace.clone(), module.id);
            modules
                .by_key
                .insert(module.id, (*typed.id(), typed.exports(), types));
            analyzed.insert(module.id, module.typed(typed));
        }

        analyzed
            .into_values()
            .for_each(|x| state.modules.insert(x.id, x));

        (state, Analyzed(ids))
    }
}

#[cfg(test)]
mod tests {
    use crate::{Analyzed, Context, Engine, Input};
    use assert_fs::{
        prelude::{FileWriteStr, PathChild},
        TempDir,
    };
    use kore::assert_eq;
    use lang::ModuleId;
    use std::collections::HashSet;

    #[derive(Clone, Copy, PartialEq, Eq, Hash)]
    pub struct MockLibrary;

    #[test]
    fn analyze_one_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("const FOO = 123;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse().link().analyze();

        let (state, Analyzed(analyzed)) = engine.execute(&plan, &input);

        let id = ModuleId(0);
        assert_eq!(analyzed, HashSet::from([id]));

        let main = state.modules.get_by_id(&id).unwrap();
        insta::assert_debug_snapshot!(main.typed);
    }

    #[test]
    fn analyze_multiple_files() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str(
                "use @/foo/foo;
const MAIN = foo.FOO;",
            )
            .unwrap();
        root_dir
            .child("foo/foo.kn")
            .write_str(
                "use ./bar;
const FOO = bar.BAR;",
            )
            .unwrap();
        root_dir
            .child("foo/bar.kn")
            .write_str("const BAR = 456;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse_and_traverse().link().analyze();

        let (state, Analyzed(analyzed)) = engine.execute(&plan, &input);

        let main_id = ModuleId(0);
        let foo_id = ModuleId(1);
        let bar_id = ModuleId(2);
        assert_eq!(analyzed, HashSet::from([main_id, foo_id, bar_id]));

        let bar = state.modules.get_by_id(&main_id).unwrap();
        insta::assert_debug_snapshot!(bar.typed);

        let foo = state.modules.get_by_id(&main_id).unwrap();
        insta::assert_debug_snapshot!(foo.typed);

        let main = state.modules.get_by_id(&main_id).unwrap();
        insta::assert_debug_snapshot!(main.typed);
    }
}
