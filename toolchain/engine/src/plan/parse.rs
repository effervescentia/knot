use crate::{state::Module, State};
use kore::{pipeline::Transform, uniqueue::Uniqueue};
use lang::{ast, ModuleId};
use std::{
    collections::HashSet,
    path::{Path, PathBuf},
};

pub struct Parsed(pub HashSet<ModuleId>);

pub struct Options {
    /// true if imports should be followed
    pub follow_imports: bool,
}

pub struct Parse<Tx>(Tx, Options);

impl<Tx> Parse<Tx> {
    pub const fn bind(options: Options) -> impl FnOnce(Tx) -> Self {
        |tx| Self(tx, options)
    }

    fn extract_dependencies(&self, ast: &ast::raw::Program, relative_to: &Path) -> Vec<PathBuf> {
        self.1
            .follow_imports
            .then(|| ast.get_dependencies(relative_to))
            .unwrap_or_default()
    }
}

impl<'a, Tx, Log> Transform for Parse<Tx>
where
    Tx: Transform<Context = State<'a, Log>, Out = ()>,
    Log: 'a,
{
    type Context = Tx::Context;
    type In = Tx::In;
    type Out = Parsed;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        let (mut state, ()) = self.0.apply(input);

        let mut queue = Uniqueue::from(state.scope.clone());
        let mut parsed = HashSet::new();

        while let Some(path) = queue.pop() {
            let id = state.modules.register(&path);
            let (text, ast) = state.load_and_parse_module(&path);
            let dependencies = self.extract_dependencies(&ast, &path);

            state.modules.insert(id, Module::raw(id, path, text, ast));
            parsed.insert(id);

            for dependency in dependencies {
                queue.push(dependency);
            }
        }

        (state, Parsed(parsed))
    }
}

#[cfg(test)]
mod tests {
    use super::Parsed;
    use crate::{state::Status, Context, Engine, Input};
    use assert_fs::{
        prelude::{FileTouch, FileWriteStr, PathChild},
        TempDir,
    };
    use kore::{assert_eq, str};
    use lang::ModuleId;
    use std::{collections::HashSet, path::PathBuf};

    #[derive(Clone, Copy, PartialEq, Eq, Hash)]
    pub struct MockLibrary;

    #[test]
    fn parse_one_empty_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir.child("main.kn").touch().unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId(0)]));

        let module = state.modules.get_by_id(&ModuleId(0)).unwrap();

        assert_eq!(module.id, ModuleId(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, str!(""));
        assert_eq!(module.status, Status::Active);
        insta::assert_debug_snapshot!(module.raw);
    }

    #[test]
    fn parse_one_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("const FOO = 123;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_entry("main.kn", [MockLibrary]);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId(0)]));

        let module = state.modules.get_by_id(&ModuleId(0)).unwrap();

        assert_eq!(module.id, ModuleId(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, str!("const FOO = 123;"));
        assert_eq!(module.status, Status::Active);
        insta::assert_debug_snapshot!(module.raw);
    }

    #[test]
    fn parse_multiple_files_from_glob() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("const ROOT = true;")
            .unwrap();
        root_dir
            .child("foo/foo.kn")
            .write_str("const FOO = 123;")
            .unwrap();
        root_dir
            .child("bar/bar.kn")
            .write_str("const BAR = 456;")
            .unwrap();

        let engine = Engine::new(Context::new(&root_dir, false));
        let input = Input::from_glob("**/*.kn", [MockLibrary]);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(
            parsed,
            HashSet::from([ModuleId(0), ModuleId(1), ModuleId(2)])
        );

        let bar = state.modules.get_by_id(&ModuleId(0)).unwrap();

        assert_eq!(bar.id, ModuleId(0));
        assert_eq!(bar.path, PathBuf::from("bar/bar.kn"));
        assert_eq!(bar.text, str!("const BAR = 456;"));
        assert_eq!(bar.status, Status::Active);
        insta::assert_debug_snapshot!(bar.raw);

        let foo = state.modules.get_by_id(&ModuleId(1)).unwrap();

        assert_eq!(foo.id, ModuleId(1));
        assert_eq!(foo.path, PathBuf::from("foo/foo.kn"));
        assert_eq!(foo.text, str!("const FOO = 123;"));
        assert_eq!(foo.status, Status::Active);
        insta::assert_debug_snapshot!(foo.raw);

        let main = state.modules.get_by_id(&ModuleId(2)).unwrap();

        assert_eq!(main.id, ModuleId(2));
        assert_eq!(main.path, PathBuf::from("main.kn"));
        assert_eq!(main.text, str!("const ROOT = true;"));
        assert_eq!(main.status, Status::Active);
        insta::assert_debug_snapshot!(main.raw);
    }

    #[test]
    fn parse_multiple_files_from_entrypoint() {
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
        let plan = Engine::plan().parse_and_traverse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(
            parsed,
            HashSet::from([ModuleId(0), ModuleId(1), ModuleId(2)])
        );

        let main = state.modules.get_by_id(&ModuleId(0)).unwrap();

        assert_eq!(main.id, ModuleId(0));
        assert_eq!(main.path, PathBuf::from("main.kn"));
        assert_eq!(main.text, str!("use @/foo/foo;"));
        assert_eq!(main.status, Status::Active);
        insta::assert_debug_snapshot!(main.raw);

        let foo = state.modules.get_by_id(&ModuleId(1)).unwrap();

        assert_eq!(foo.id, ModuleId(1));
        assert_eq!(foo.path, PathBuf::from("foo/foo.kn"));
        assert_eq!(foo.text, str!("use ./bar;"));
        assert_eq!(foo.status, Status::Active);
        insta::assert_debug_snapshot!(foo.raw);

        let bar = state.modules.get_by_id(&ModuleId(2)).unwrap();

        assert_eq!(bar.id, ModuleId(2));
        assert_eq!(bar.path, PathBuf::from("foo/bar.kn"));
        assert_eq!(bar.text, str!("const BAR = 456;"));
        assert_eq!(bar.status, Status::Active);
        insta::assert_debug_snapshot!(bar.raw);
    }
}
