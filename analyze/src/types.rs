use super::{context::StrongContext, PreviewType, RefKind};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,

    Enumerated(Vec<(String, Vec<T>)>),
    EnumeratedVariant(Vec<T>, T),
    EnumeratedInstance(T),

    Function(Vec<T>, T),
    View(Vec<T>),
    Module(Vec<(String, RefKind, T)>),
}

impl<T> Type<T> {
    pub fn opt_map<R>(&self, f: &impl Fn(&T) -> Option<R>) -> Option<Type<R>> {
        match self {
            Self::Nil => Some(Type::Nil),
            Self::Boolean => Some(Type::Boolean),
            Self::Integer => Some(Type::Integer),
            Self::Float => Some(Type::Float),
            Self::String => Some(Type::String),
            Self::Style => Some(Type::Style),
            Self::Element => Some(Type::Element),

            Self::Enumerated(variants) => Some(Type::Enumerated(
                variants
                    .iter()
                    .map(|(name, parameters)| {
                        Some((
                            name.clone(),
                            parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                        ))
                    })
                    .collect::<Option<Vec<_>>>()?,
            )),

            Self::EnumeratedVariant(parameters, x) => Some(Type::EnumeratedVariant(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(x)?,
            )),

            Self::EnumeratedInstance(x) => Some(Type::EnumeratedInstance(f(x)?)),

            Self::Function(parameters, body) => Some(Type::Function(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(body)?,
            )),

            Self::View(parameters) => Some(Type::View(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
            )),

            Self::Module(entries) => Some(Type::Module(
                entries
                    .iter()
                    .map(|(name, kind, x)| Some((name.clone(), kind.clone(), f(x)?)))
                    .collect::<Option<Vec<_>>>()?,
            )),
        }
    }
}

impl Type<usize> {
    pub fn preview(&self, kind: &RefKind, ctx: &StrongContext) -> Option<PreviewType> {
        self.opt_map(&|x| match ctx.get_strong(x, kind) {
            Some(Ok(typ)) => typ.preview(kind, ctx).map(Box::new),
            _ => None,
        })
        .map(PreviewType)
    }
}

#[cfg(test)]
mod tests {
    use crate::{types::Type, RefKind};

    #[test]
    fn opt_map_primitive() {
        let map = |_: &usize| Some(());

        assert_eq!(Type::Nil::<usize>.opt_map(&map), Some(Type::Nil));
        assert_eq!(Type::Boolean::<usize>.opt_map(&map), Some(Type::Boolean));
        assert_eq!(Type::Integer::<usize>.opt_map(&map), Some(Type::Integer));
        assert_eq!(Type::Float::<usize>.opt_map(&map), Some(Type::Float));
        assert_eq!(Type::String::<usize>.opt_map(&map), Some(Type::String));
        assert_eq!(Type::Style::<usize>.opt_map(&map), Some(Type::Style));
        assert_eq!(Type::Element::<usize>.opt_map(&map), Some(Type::Element));
    }

    #[test]
    fn opt_map_enumerated_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::Enumerated::<usize>(vec![
                (String::from("Foo"), vec![0, 1]),
                (String::from("Bar"), vec![2])
            ])
            .opt_map(&map),
            Some(Type::Enumerated(vec![
                (String::from("Foo"), vec![(), ()]),
                (String::from("Bar"), vec![()])
            ]))
        );
    }

    #[test]
    fn opt_map_enumerated_none() {
        let map = |_: &usize| None::<()>;

        assert_eq!(
            Type::Enumerated::<usize>(vec![
                (String::from("Foo"), vec![0, 1]),
                (String::from("Bar"), vec![2])
            ])
            .opt_map(&map),
            None
        );
    }

    #[test]
    fn opt_map_enumerated_variant_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::EnumeratedVariant::<usize>(vec![0, 1], 2).opt_map(&map),
            Some(Type::EnumeratedVariant(vec![(), ()], ()))
        );
    }

    #[test]
    fn opt_map_enumerated_variant_none_from_parameters() {
        let bad_type = 0;
        let map = |x: &usize| if x == &bad_type { None } else { Some(()) };

        assert_eq!(
            Type::EnumeratedVariant::<usize>(vec![0, 1], 2).opt_map(&map),
            None
        );
    }

    #[test]
    fn opt_map_enumerated_variant_none_from_result() {
        let bad_type = 2;
        let map = |x: &usize| if x == &bad_type { None } else { Some(()) };

        assert_eq!(
            Type::EnumeratedVariant::<usize>(vec![0, 1], 2).opt_map(&map),
            None
        );
    }

    #[test]
    fn opt_map_enumerated_instance_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::EnumeratedInstance::<usize>(0).opt_map(&map),
            Some(Type::EnumeratedInstance(()))
        );
    }

    #[test]
    fn opt_map_enumerated_instance_none() {
        let map = |_: &usize| None::<()>;

        assert_eq!(Type::EnumeratedInstance::<usize>(0).opt_map(&map), None);
    }

    #[test]
    fn opt_map_function_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::Function::<usize>(vec![0, 1], 2).opt_map(&map),
            Some(Type::Function(vec![(), ()], ()))
        );
    }

    #[test]
    fn opt_map_function_none_from_parameters() {
        let bad_type = 0;
        let map = |x: &usize| if x == &bad_type { None } else { Some(()) };

        assert_eq!(Type::Function::<usize>(vec![0, 1], 2).opt_map(&map), None);
    }

    #[test]
    fn opt_map_function_none_from_result() {
        let bad_type = 2;
        let map = |x: &usize| if x == &bad_type { None } else { Some(()) };

        assert_eq!(Type::Function::<usize>(vec![0, 1], 2).opt_map(&map), None);
    }

    #[test]
    fn opt_map_view_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::View::<usize>(vec![0, 1]).opt_map(&map),
            Some(Type::View(vec![(), ()]))
        );
    }

    #[test]
    fn opt_map_view_none() {
        let map = |_: &usize| None::<()>;

        assert_eq!(Type::View::<usize>(vec![0, 1]).opt_map(&map), None);
    }

    #[test]
    fn opt_map_module_some() {
        let map = |_: &usize| Some(());

        assert_eq!(
            Type::Module::<usize>(vec![
                (String::from("foo"), RefKind::Value, 0),
                (String::from("bar"), RefKind::Type, 1),
                (String::from("fizz"), RefKind::Mixed, 2)
            ])
            .opt_map(&map),
            Some(Type::Module(vec![
                (String::from("foo"), RefKind::Value, ()),
                (String::from("bar"), RefKind::Type, ()),
                (String::from("fizz"), RefKind::Mixed, ())
            ]))
        );
    }

    #[test]
    fn opt_map_module_none() {
        let map = |_: &usize| None::<()>;

        assert_eq!(
            Type::Module::<usize>(vec![
                (String::from("foo"), RefKind::Value, 0),
                (String::from("bar"), RefKind::Type, 1),
                (String::from("fizz"), RefKind::Mixed, 2)
            ])
            .opt_map(&map),
            None
        );
    }
}
