use super::parse::Parsed;
use crate::{validate::Validator, State};
use kore::pipeline::Transform;
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
    Tx: Transform<Context = State<'a, Log>, Out = Res>,
    Res: Into<Parsed>,
    Log: 'a,
{
    type Context = Tx::Context;
    type In = Tx::In;
    type Out = Linked;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        let (mut state, result) = self.0.apply(input);
        let Parsed(ids) = result.into();

        for id in &ids {
            if let Some(module) = state.modules.get_by_id(id) {
                let dependencies = module.raw.get_dependencies(&module.path);

                for dependency in &dependencies {
                    if let Some(dependency_id) = state.modules.get_id_by_path(dependency) {
                        state.modules.add_dependency(id, &dependency_id);
                    } else {
                        panic!("replace this with an actual error");
                    }
                }
            } else {
                panic!("replace this with an actual error");
            }
        }

        Validator.validate(&state);

        (state, Linked(ids))
    }
}

#[cfg(test)]
mod tests {
    use crate::{Context, Engine, Input, Linked};
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
    fn link_one_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("const FOO = 123;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse().link();

        let (state, Linked(linked)) = engine.execute(&plan, &input);

        let id = ModuleId(0);
        assert_eq!(linked, HashSet::from([id]));
        assert_eq!(state.modules.dependencies().edges().count(), 0);
    }

    #[test]
    fn link_multiple_files() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("use @/foo/foo;")
            .unwrap();
        root_dir
            .child("foo/foo.kn")
            .write_str("use ./bar;")
            .unwrap();
        root_dir
            .child("foo/bar.kn")
            .write_str("const BAR = 456;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse_and_traverse().link();

        let (state, Linked(linked)) = engine.execute(&plan, &input);

        let main_id = ModuleId(0);
        let foo_id = ModuleId(1);
        let bar_id = ModuleId(2);
        assert_eq!(linked, HashSet::from([main_id, foo_id, bar_id]));
        assert_eq!(
            state.modules.dependencies().edges().collect::<HashSet<_>>(),
            HashSet::from([(main_id, foo_id), (foo_id, bar_id)])
        );
    }
}
