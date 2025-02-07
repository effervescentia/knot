use lang::ast;

use crate::engine2::{input, pipeline::Transform, state::ModuleId, Scope, State};
use std::{collections::HashSet, fs, path::Path};

pub struct Parsed(HashSet<ModuleId>);

pub struct Parse<Tx>(pub Tx);

impl<Tx> Parse<Tx> {
    fn load_and_parse_module<T>(path: T) -> (String, ast::meta::Program<()>)
    where
        T: AsRef<Path>,
    {
        let text = fs::read_to_string(path).unwrap();

        let (ast, _) = parse::program::parse(&text).unwrap();

        (text, ast)
    }
}

impl<'a, Tx> Transform for Parse<Tx>
where
    Tx: Transform<Out = (State<'a>, ())>,
{
    type In = Tx::In;
    type Out = (State<'a>, Parsed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (mut state, ()) = self.0.apply(input);

        let parsed = state
            .scope
            .files
            .clone()
            .into_iter()
            .map(|path| {
                println!("parsing {}", path.display());
                println!("state {:?}", state);
                let id = state.identify_path(&path).unwrap();
                let absolute = &state.context.source_dir.join(&path);
                let (text, ast) = Self::load_and_parse_module(absolute);

                state.upsert_module(id, path, text, ast);

                if state.scope.follow_imports {
                    // TODO: do some extra import walking here
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
        input::{Input, Source},
        state::{Module, ModuleId, State, Status},
        Context, Engine, Library,
    };
    use assert_fs::{
        prelude::{FileTouch, FileWriteStr, PathChild},
        TempDir,
    };
    use kore::assert_eq;
    use lang::ast;
    use std::{
        collections::HashSet,
        path::{Path, PathBuf},
    };

    #[test]
    fn parse_one_empty_file() -> Result<(), Box<dyn std::error::Error>> {
        let root_dir = TempDir::new()?;

        root_dir.child("main.kn").touch()?;

        let context = Context::new(&root_dir, false);
        let engine = Engine::new(context);
        let input = Input::from_entry("main.kn", []);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId::from(0)]));

        let module = state.get_module(&ModuleId::from(0)).unwrap();

        assert_eq!(module.id, ModuleId::from(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, String::from(""));
        assert_eq!(module.status, Status::Pending);
        insta::assert_debug_snapshot!(module.ast);

        Ok(())
    }

    #[test]
    fn parse_one_file() -> Result<(), Box<dyn std::error::Error>> {
        let root_dir = TempDir::new()?;

        root_dir.child("main.kn").write_str("const FOO = 123;")?;

        let context = Context::new(&root_dir, false);
        let engine = Engine::new(context);
        let input = Input::from_entry("main.kn", []);
        let plan = Engine::plan().parse();

        let (state, Parsed(parsed)) = engine.execute(&plan, &input);

        assert_eq!(parsed, HashSet::from([ModuleId::from(0)]));

        let module = state.get_module(&ModuleId::from(0)).unwrap();

        assert_eq!(module.id, ModuleId::from(0));
        assert_eq!(module.path, PathBuf::from("main.kn"));
        assert_eq!(module.text, String::from("const FOO = 123;"));
        assert_eq!(module.status, Status::Pending);
        insta::assert_debug_snapshot!(module.ast);

        Ok(())
    }

    #[test]
    fn parse_multiple_files() -> Result<(), Box<dyn std::error::Error>> {
        let root_dir = TempDir::new()?;

        root_dir.child("main.kn").write_str("const ROOT = true;")?;
        root_dir.child("foo/foo.kn").write_str("const FOO = 123;")?;
        root_dir.child("bar/bar.kn").write_str("const BAR = 456;")?;

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
        assert_eq!(bar.text, String::from("const BAR = 456;"));
        assert_eq!(bar.status, Status::Pending);
        insta::assert_debug_snapshot!(bar.ast);

        let foo = state.get_module(&ModuleId::from(1)).unwrap();

        assert_eq!(foo.id, ModuleId::from(1));
        assert_eq!(foo.path, PathBuf::from("foo/foo.kn"));
        assert_eq!(foo.text, String::from("const FOO = 123;"));
        assert_eq!(foo.status, Status::Pending);
        insta::assert_debug_snapshot!(foo.ast);

        let main = state.get_module(&ModuleId::from(2)).unwrap();

        assert_eq!(main.id, ModuleId::from(2));
        assert_eq!(main.path, PathBuf::from("main.kn"));
        assert_eq!(main.text, String::from("const ROOT = true;"));
        assert_eq!(main.status, Status::Pending);
        insta::assert_debug_snapshot!(main.ast);

        Ok(())
    }
}
