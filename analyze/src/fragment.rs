use super::infer::weak::{ToWeak, WeakRef};
use lang::ast::{
    Declaration, Expression, Import, ImportTarget, Module, Parameter, Statement, TypeExpression,
    KSX,
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
#[allow(clippy::upper_case_acronyms)]
pub enum Fragment {
    Expression(Expression<usize, usize, usize>),
    Statement(Statement<usize>),
    KSX(KSX<usize, usize>),
    Parameter(Parameter<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize, usize>),
    Import(Import),
    Module(Module<usize, usize>),
}

impl Fragment {
    pub fn to_binding(&self) -> Vec<String> {
        match self {
            Self::Statement(Statement::Variable(name, ..))
            | Self::Parameter(Parameter { name, .. }) => vec![name.clone()],

            Self::Declaration(x) => vec![x.name().clone()],

            Self::Import(Import {
                path,
                aliases: None,
                ..
            }) => vec![path
                .last()
                .expect("failed to get last part of import path")
                .clone()],

            Self::Import(Import {
                path,
                aliases: Some(aliases),
                ..
            }) => aliases
                .iter()
                .map(|(target, alias)| match target {
                    ImportTarget::Module => alias
                        .as_ref()
                        .or_else(|| path.last())
                        .expect("failed to get last part of import path")
                        .clone(),

                    ImportTarget::Named(name) => alias.as_ref().unwrap_or(name).clone(),
                })
                .collect(),

            _ => vec![],
        }
    }
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::KSX(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use kore::assert_eq;
    use lang::{
        ast::{Import, ImportSource, ImportTarget, Statement},
        test::fixture as f,
    };

    #[test]
    fn binding_variable() {
        assert_eq!(
            Fragment::Statement(Statement::Variable(String::from("foo"), 0)).to_binding(),
            vec![String::from("foo")]
        );
    }

    #[test]
    fn binding_type_alias() {
        assert_eq!(
            Fragment::Declaration(f::a::type_("Foo", 0)).to_binding(),
            vec![String::from("Foo")]
        );
    }

    #[test]
    fn binding_enumerated() {
        assert_eq!(
            Fragment::Declaration(f::a::enum_("Foo", vec![])).to_binding(),
            vec![String::from("Foo")]
        );
    }

    #[test]
    fn binding_constant() {
        assert_eq!(
            Fragment::Declaration(f::a::const_("FOO", None, 0)).to_binding(),
            vec![String::from("FOO")]
        );
    }

    #[test]
    fn binding_function() {
        assert_eq!(
            Fragment::Declaration(f::a::func_("foo", vec![], None, 0)).to_binding(),
            vec![String::from("foo")]
        );
    }

    #[test]
    fn binding_view() {
        assert_eq!(
            Fragment::Declaration(f::a::view("Foo", vec![], 0)).to_binding(),
            vec![String::from("Foo")]
        );
    }

    #[test]
    fn binding_module_import() {
        assert_eq!(
            Fragment::Import(Import {
                source: ImportSource::Local,
                path: vec![String::from("foo"), String::from("bar")],
                aliases: None
            })
            .to_binding(),
            vec![String::from("bar")]
        );
    }

    #[test]
    fn binding_module_star_import() {
        assert_eq!(
            Fragment::Import(Import {
                source: ImportSource::Local,
                path: vec![String::from("foo"), String::from("bar")],
                aliases: Some(vec![(ImportTarget::Module, None)])
            })
            .to_binding(),
            vec![String::from("bar")]
        );
    }

    #[test]
    fn binding_module_import_with_alias() {
        assert_eq!(
            Fragment::Import(Import {
                source: ImportSource::Local,
                path: vec![String::from("foo"), String::from("bar")],
                aliases: Some(vec![(ImportTarget::Module, Some(String::from("fizz")))])
            })
            .to_binding(),
            vec![String::from("fizz")]
        );
    }

    #[test]
    fn binding_module() {
        assert_eq!(
            Fragment::Declaration(f::a::module("foo", 0)).to_binding(),
            vec![String::from("foo")]
        );
    }
}
