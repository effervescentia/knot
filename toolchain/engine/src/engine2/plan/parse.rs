use crate::engine2::{pipeline::Transform, State};
use kore::uniqueue::Uniqueue;
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
    Tx: Transform<Out = (State<'a, Log>, ())>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Parsed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (mut state, ()) = self.0.apply(input);

        let mut queue = Uniqueue::from(state.scope.clone());
        let mut parsed = HashSet::new();

        while let Some(path) = queue.pop() {
            let id = state.identify_path(&path).unwrap();
            let (text, ast) = state.load_and_parse_module(&path);
            let dependencies = self.extract_dependencies(&ast, &path);

            state.upsert_module(id, path, text, ast);
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
    use crate::engine2::{input::Input, state::Status, Context, Engine};
    use assert_fs::{
        prelude::{FileTouch, FileWriteStr, PathChild},
        TempDir,
    };
    use kore::{assert_eq, str};
    use lang::ModuleId;
    use std::{collections::HashSet, path::PathBuf};

    #[test]
    fn parse_one_empty_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir.child("main.kn").touch().unwrap();

        let context = Context::new(&root_dir, false);
        let engine = Engine::new(context);
        let input = Input::from_entry("main.kn", []);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId(0)]));

        let module = state.get_module(&ModuleId(0)).unwrap();

        assert_eq!(module.id, ModuleId(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, str!(""));
        assert_eq!(module.status, Status::Pending);
        insta::assert_debug_snapshot!(module.ast);
    }

    #[test]
    fn parse_one_file() {
        let root_dir = TempDir::new().unwrap();

        root_dir
            .child("main.kn")
            .write_str("const FOO = 123;")
            .unwrap();

        let context = Context::new(&root_dir, false);
        let engine = Engine::new(context);
        let input = Input::from_entry("main.kn", []);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId(0)]));

        let module = state.get_module(&ModuleId(0)).unwrap();

        assert_eq!(module.id, ModuleId(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, str!("const FOO = 123;"));
        assert_eq!(module.status, Status::Pending);
        insta::assert_debug_snapshot!(module.ast);
    }

    #[test]
    fn parse_multiple_files() {
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

        let context = Context::new(&root_dir, false);
        let engine = Engine::new(context);
        let input = Input::from_glob("**/*.kn", []);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(
            parsed,
            HashSet::from([ModuleId(0), ModuleId(1), ModuleId(2)])
        );

        let bar = state.get_module(&ModuleId(0)).unwrap();

        assert_eq!(bar.id, ModuleId(0));
        assert_eq!(bar.path, PathBuf::from("bar/bar.kn"));
        assert_eq!(bar.text, str!("const BAR = 456;"));
        assert_eq!(bar.status, Status::Pending);
        insta::assert_debug_snapshot!(bar.ast);

        let foo = state.get_module(&ModuleId(1)).unwrap();

        assert_eq!(foo.id, ModuleId(1));
        assert_eq!(foo.path, PathBuf::from("foo/foo.kn"));
        assert_eq!(foo.text, str!("const FOO = 123;"));
        assert_eq!(foo.status, Status::Pending);
        insta::assert_debug_snapshot!(foo.ast);

        let main = state.get_module(&ModuleId(2)).unwrap();

        assert_eq!(main.id, ModuleId(2));
        assert_eq!(main.path, PathBuf::from("main.kn"));
        assert_eq!(main.text, str!("const ROOT = true;"));
        assert_eq!(main.status, Status::Pending);
        insta::assert_debug_snapshot!(main.ast);
    }
}
