use crate::engine2::{pipeline::Transform, state::ModuleId, State};
use lang::ast;
use std::{collections::HashSet, fs, path::Path};

pub struct Parsed(pub HashSet<ModuleId>);

pub struct Options {
    /** true if imports should be followed */
    pub follow_imports: bool,
}

pub struct Parse<Tx>(Tx, Options);

impl<Tx> Parse<Tx> {
    pub const fn bind(options: Options) -> impl FnOnce(Tx) -> Self {
        |tx| Self(tx, options)
    }

    fn load_and_parse_module<T>(path: T) -> (String, ast::raw::Program)
    where
        T: AsRef<Path>,
    {
        let text = fs::read_to_string(path).unwrap();

        let (ast, _) = parse::program::parse(&text).unwrap();

        (text, ast)
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

        let parsed = state
            .scope
            .clone()
            .into_iter()
            .map(|path| {
                let id = state.identify_path(&path).unwrap();
                let absolute = state.get_absolute_path(&path);
                let (text, ast) = Self::load_and_parse_module(absolute);

                state.upsert_module(id, path, text, ast);

                if self.1.follow_imports {
                    // TODO: walk and queue imported modules
                    // for link in ast.to_links(&link) {
                    //     if !parsed.has_by_link(&link) && !link.is_library() {
                    //         visitor.queue(link);
                    //     }
                    // }
                }

                id
            })
            .collect();

        (state, Parsed(parsed))
    }
}

#[cfg(test)]
mod tests {
    use super::Parsed;
    use crate::engine2::{
        input::Input,
        state::{ModuleId, Status},
        Context, Engine,
    };
    use assert_fs::{
        prelude::{FileTouch, FileWriteStr, PathChild},
        TempDir,
    };
    use kore::{assert_eq, str};
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

        assert_eq!(parsed, HashSet::from([ModuleId::from(0)]));

        let module = state.get_module(&ModuleId::from(0)).unwrap();

        assert_eq!(module.id, ModuleId::from(0));
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

        assert_eq!(parsed, HashSet::from([ModuleId::from(0)]));

        let module = state.get_module(&ModuleId::from(0)).unwrap();

        assert_eq!(module.id, ModuleId::from(0));
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
            HashSet::from([ModuleId::from(0), ModuleId::from(1), ModuleId::from(2)])
        );

        let bar = state.get_module(&ModuleId::from(0)).unwrap();

        assert_eq!(bar.id, ModuleId::from(0));
        assert_eq!(bar.path, PathBuf::from("bar/bar.kn"));
        assert_eq!(bar.text, str!("const BAR = 456;"));
        assert_eq!(bar.status, Status::Pending);
        insta::assert_debug_snapshot!(bar.ast);

        let foo = state.get_module(&ModuleId::from(1)).unwrap();

        assert_eq!(foo.id, ModuleId::from(1));
        assert_eq!(foo.path, PathBuf::from("foo/foo.kn"));
        assert_eq!(foo.text, str!("const FOO = 123;"));
        assert_eq!(foo.status, Status::Pending);
        insta::assert_debug_snapshot!(foo.ast);

        let main = state.get_module(&ModuleId::from(2)).unwrap();

        assert_eq!(main.id, ModuleId::from(2));
        assert_eq!(main.path, PathBuf::from("main.kn"));
        assert_eq!(main.text, str!("const ROOT = true;"));
        assert_eq!(main.status, Status::Pending);
        insta::assert_debug_snapshot!(main.ast);
    }
}
