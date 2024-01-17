use std::collections::HashMap;

use kore::invariant;

use super::raw;
use super::typed;
use super::Range;

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct NodeId(pub usize);

pub struct Visitor<'a, T, C> {
    pub expression: &'a dyn Fn(super::Expression<T, T, T>, C) -> (T, C),
    pub statement: &'a dyn Fn(super::Statement<T>, C) -> (T, C),
    pub component: &'a dyn Fn(super::Component<T, T>, C) -> (T, C),
    pub type_expression: &'a dyn Fn(super::TypeExpression<T>, C) -> (T, C),
    pub parameter: &'a dyn Fn(super::Parameter<T, T>, C) -> (T, C),
    pub declaration: &'a dyn Fn(super::Declaration<T, T, T, T>, C) -> (T, C),
    pub import: &'a dyn Fn(super::Import, C) -> (T, C),
    pub module: &'a dyn Fn(super::Module<T, T>, C) -> (T, C),
}

impl<'a, T, C> Visitor<'a, T, C> {
    fn expression(&self, x: super::Expression<T, T, T>, ctx: C) -> (T, C) {
        (self.expression)(x, ctx)
    }

    fn statement(&self, x: super::Statement<T>, ctx: C) -> (T, C) {
        (self.statement)(x, ctx)
    }

    fn component(&self, x: super::Component<T, T>, ctx: C) -> (T, C) {
        (self.component)(x, ctx)
    }

    fn type_expression(&self, x: super::TypeExpression<T>, ctx: C) -> (T, C) {
        (self.type_expression)(x, ctx)
    }

    fn parameter(&self, x: super::Parameter<T, T>, ctx: C) -> (T, C) {
        (self.parameter)(x, ctx)
    }

    fn declaration(&self, x: super::Declaration<T, T, T, T>, ctx: C) -> (T, C) {
        (self.declaration)(x, ctx)
    }

    fn import(&self, x: super::Import, ctx: C) -> (T, C) {
        (self.import)(x, ctx)
    }

    fn module(&self, x: super::Module<T, T>, ctx: C) -> (T, C) {
        (self.module)(x, ctx)
    }

    fn visit_each<Target>(&'a self, xs: Vec<Target>, c: C) -> (Vec<T>, C)
    where
        Target: Visit<Visitor<'a, T, C> = &'a Self>,
    {
        xs.into_iter().fold((vec![], c), |(mut acc, c), x| {
            let (x, c) = x.visit(self, c);
            acc.push(x);
            (acc, c)
        })
    }

    fn visit_values<Key, Target>(&'a self, xs: Vec<(Key, Target)>, c: C) -> (Vec<(Key, T)>, C)
    where
        Target: Visit<Visitor<'a, T, C> = &'a Self>,
    {
        xs.into_iter()
            .fold((vec![], c), |(mut acc, c), (key, value)| {
                let (value, c) = value.visit(self, c);
                acc.push((key, value));
                (acc, c)
            })
    }

    fn visit_option<Target>(&'a self, x: Option<Target>, c: C) -> (Option<T>, C)
    where
        Target: Visit<Visitor<'a, T, C> = &'a Self>,
    {
        if let Some(x) = x {
            let (x, c) = x.visit(self, c);
            (Some(x), c)
        } else {
            (None, c)
        }
    }

    fn visit_optional_values<Key, Target>(
        &'a self,
        xs: Vec<(Key, Option<Target>)>,
        c: C,
    ) -> (Vec<(Key, Option<T>)>, C)
    where
        Target: Visit<Visitor<'a, T, C> = &'a Self>,
    {
        xs.into_iter()
            .fold((vec![], c), |(mut acc, c), (key, value)| {
                let (value, c) = self.visit_option(value, c);
                acc.push((key, value));
                (acc, c)
            })
    }
}

struct AugmentContext<'a> {
    next_id: usize,
    types: HashMap<NodeId, typed::TypeRef<'a>>,
}

impl<'a> AugmentContext<'a> {
    fn next(&mut self) -> typed::TypeRef<'a> {
        let id = NodeId(self.next_id);
        self.next_id += 1;

        self.types
            .remove(&id)
            .unwrap_or_else(|| invariant!("type does not exist by id {id:?}"))
    }

    fn wrap<Value, Result, F>(mut self, value: Value, range: Range, f: F) -> (Result, Self)
    where
        F: Fn(typed::Node<Value, typed::TypeRef<'a>>) -> Result,
    {
        (f(typed::Node::new(value, self.next(), range)), self)
    }
}

trait Augment {
    type Node<Type>;

    fn augment(self, ctx: AugmentContext) -> (Self::Node<typed::TypeRef>, AugmentContext);
}

pub trait Visit {
    type Result<T>;

    type Visitor<'a, T, C>
    where
        T: 'a,
        C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context);
}

impl Visit for raw::Expression {
    type Result<T> = super::Expression<T, T, T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C> where T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Expression::Primitive(x) => v.expression(super::Expression::Primitive(x), c),
            super::Expression::Identifier(x) => v.expression(super::Expression::Identifier(x), c),

            /* containers */
            super::Expression::Group(x) => {
                let (x, c) = x.visit(v, c);

                v.expression(super::Expression::Group(Box::new(x)), c)
            }
            super::Expression::Closure(xs) => {
                let (xs, c) = v.visit_each(xs, c);

                v.expression(super::Expression::Closure(xs), c)
            }

            /* operations */
            super::Expression::UnaryOperation(op, x) => {
                let (x, c) = x.visit(v, c);

                v.expression(super::Expression::UnaryOperation(op, Box::new(x)), c)
            }
            super::Expression::BinaryOperation(op, l, r) => {
                let (l, c) = l.visit(v, c);
                let (r, c) = r.visit(v, c);

                v.expression(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    c,
                )
            }
            super::Expression::PropertyAccess(x, property) => {
                let (x, c) = x.visit(v, c);

                v.expression(super::Expression::PropertyAccess(Box::new(x), property), c)
            }
            super::Expression::FunctionCall(x, arguments) => {
                let (x, c) = x.visit(v, c);
                let (arguments, c) = v.visit_each(arguments, c);

                v.expression(super::Expression::FunctionCall(Box::new(x), arguments), c)
            }

            /* domain syntax */
            super::Expression::Style(rules) => {
                let (rules, c) = v.visit_values(rules, c);

                v.expression(super::Expression::Style(rules), c)
            }
            super::Expression::Component(x) => {
                let (x, c) = x.visit(v, c);

                v.expression(super::Expression::Component(Box::new(x)), c)
            }
        }
    }
}

impl Augment for raw::Expression {
    type Node<Type> = typed::Expression<Type>;

    fn augment(self, ctx: AugmentContext) -> (Self::Node<typed::TypeRef>, AugmentContext) {
        let raw::Node { value, range } = self.0;
        // let wrap = |x| ctx.wrap(x, range, typed::Expression);

        match value {
            super::Expression::Primitive(x) => {
                ctx.wrap(super::Expression::Primitive(x), range, typed::Expression)
            }
            super::Expression::Identifier(x) => {
                ctx.wrap(super::Expression::Identifier(x), range, typed::Expression)
            }

            /* containers */
            super::Expression::Group(x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(
                    super::Expression::Group(Box::new(x)),
                    range,
                    typed::Expression,
                )
            }
            super::Expression::Closure(xs) => {
                let (xs, ctx) = xs.into_iter().fold((vec![], ctx), |(mut acc, ctx), x| {
                    let (id, ctx) = x.augment(ctx);
                    acc.push(id);
                    (acc, ctx)
                });

                ctx.wrap(super::Expression::Closure(xs), range, typed::Expression)
            }

            /* operations */
            super::Expression::UnaryOperation(op, x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(
                    super::Expression::UnaryOperation(op, Box::new(x)),
                    range,
                    typed::Expression,
                )
            }
            super::Expression::BinaryOperation(op, l, r) => {
                let (l, ctx) = l.augment(ctx);
                let (r, ctx) = r.augment(ctx);

                ctx.wrap(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    range,
                    typed::Expression,
                )
            }
            super::Expression::PropertyAccess(x, property) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(
                    super::Expression::PropertyAccess(Box::new(x), property),
                    range,
                    typed::Expression,
                )
            }
            super::Expression::FunctionCall(x, arguments) => {
                let (x, ctx) = (*x).augment(ctx);
                let (arguments, ctx) =
                    arguments
                        .into_iter()
                        .fold((vec![], ctx), |(mut acc, ctx), argument| {
                            let (id, ctx) = argument.augment(ctx);
                            acc.push(id);
                            (acc, ctx)
                        });

                ctx.wrap(
                    super::Expression::FunctionCall(Box::new(x), arguments),
                    range,
                    typed::Expression,
                )
            }

            /* domain syntax */
            super::Expression::Style(rules) => {
                let (rules, ctx) =
                    rules
                        .into_iter()
                        .fold((vec![], ctx), |(mut acc, ctx), (name, x)| {
                            let (id, ctx) = x.augment(ctx);
                            acc.push((name, id));
                            (acc, ctx)
                        });

                ctx.wrap(super::Expression::Style(rules), range, typed::Expression)
            }
            super::Expression::Component(x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(
                    super::Expression::Component(Box::new(x)),
                    range,
                    typed::Expression,
                )
            }
        }
    }
}

impl Visit for raw::Statement {
    type Result<T> = super::Statement<T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C> where T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Statement::Expression(x) => {
                let (x, c) = x.visit(v, c);

                v.statement(super::Statement::Expression(x), c)
            }
            super::Statement::Variable(binding, x) => {
                let (x, c) = x.visit(v, c);

                v.statement(super::Statement::Variable(binding, x), c)
            }
        }
    }
}

impl Augment for raw::Statement {
    type Node<Type> = typed::Statement<Type>;

    fn augment(self, ctx: AugmentContext) -> (Self::Node<typed::TypeRef>, AugmentContext) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Statement::Expression(x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(super::Statement::Expression(x), range, typed::Statement)
            }
            super::Statement::Variable(binding, x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(
                    super::Statement::Variable(binding, x),
                    range,
                    typed::Statement,
                )
            }
        }
    }
}

impl Augment for raw::Component {
    type Node<Type> = typed::Component<Type>;

    fn augment(self, ctx: AugmentContext) -> (Self::Node<typed::TypeRef>, AugmentContext) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Component::Text(x) => {
                ctx.wrap(super::Component::Text(x), range, typed::Component)
            }
            super::Component::Expression(x) => {
                let (x, ctx) = x.augment(ctx);

                ctx.wrap(super::Component::Expression(x), range, typed::Component)
            }
            super::Component::Fragment(xs) => {
                let (xs, ctx) = xs.into_iter().fold((vec![], ctx), |(mut acc, ctx), x| {
                    let (id, ctx) = x.augment(ctx);
                    acc.push(id);
                    (acc, ctx)
                });

                ctx.wrap(super::Component::Fragment(xs), range, typed::Component)
            }
            super::Component::ClosedElement(tag, attributes) => {
                let (attributes, ctx) =
                    attributes
                        .into_iter()
                        .fold((vec![], ctx), |(mut acc, ctx), (key, value)| {
                            if let Some(value) = value {
                                let (id, ctx) = value.augment(ctx);
                                acc.push((key, Some(id)));
                                return (acc, ctx);
                            }

                            acc.push((key, None));
                            (acc, ctx)
                        });

                ctx.wrap(
                    super::Component::ClosedElement(tag, attributes),
                    range,
                    typed::Component,
                )
            }
            super::Component::OpenElement(start_tag, attributes, children, end_tag) => {
                let (attributes, ctx) =
                    attributes
                        .into_iter()
                        .fold((vec![], ctx), |(mut acc, ctx), (key, value)| {
                            let (result, ctx) = if let Some(value) = value {
                                let (id, ctx) = value.augment(ctx);
                                (Some(id), ctx)
                            } else {
                                (None, ctx)
                            };

                            acc.push((key, result));
                            (acc, ctx)
                        });
                let (children, ctx) =
                    children
                        .into_iter()
                        .fold((vec![], ctx), |(mut acc, ctx), x| {
                            let (id, ctx) = x.augment(ctx);
                            acc.push(id);
                            (acc, ctx)
                        });

                ctx.wrap(
                    super::Component::OpenElement(start_tag, attributes, children, end_tag),
                    range,
                    typed::Component,
                )
            }
        }
    }
}

impl Visit for raw::Component {
    type Result<T> = super::Component<T, T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Component::Text(x) => v.component(super::Component::Text(x), c),
            super::Component::Expression(x) => {
                let (x, c) = x.visit(v, c);

                v.component(super::Component::Expression(x), c)
            }
            super::Component::Fragment(xs) => {
                let (xs, c) = v.visit_each(xs, c);

                v.component(super::Component::Fragment(xs), c)
            }
            super::Component::ClosedElement(tag, attributes) => {
                let (attributes, c) = v.visit_optional_values(attributes, c);

                v.component(super::Component::ClosedElement(tag, attributes), c)
            }
            super::Component::OpenElement(start_tag, attributes, children, end_tag) => {
                let (attributes, c) = v.visit_optional_values(attributes, c);
                let (children, c) = v.visit_each(children, c);

                v.component(
                    super::Component::OpenElement(start_tag, attributes, children, end_tag),
                    c,
                )
            }
        }
    }
}

impl Visit for raw::TypeExpression {
    type Result<T> = super::TypeExpression<T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::TypeExpression::Nil => v.type_expression(super::TypeExpression::Nil, c),
            super::TypeExpression::Boolean => v.type_expression(super::TypeExpression::Boolean, c),
            super::TypeExpression::Integer => v.type_expression(super::TypeExpression::Integer, c),
            super::TypeExpression::Float => v.type_expression(super::TypeExpression::Float, c),
            super::TypeExpression::String => v.type_expression(super::TypeExpression::String, c),
            super::TypeExpression::Style => v.type_expression(super::TypeExpression::Style, c),
            super::TypeExpression::Component => {
                v.type_expression(super::TypeExpression::Component, c)
            }
            super::TypeExpression::Identifier(x) => {
                v.type_expression(super::TypeExpression::Identifier(x), c)
            }
            super::TypeExpression::Group(x) => {
                let (x, c) = x.visit(v, c);

                v.type_expression(super::TypeExpression::Group(Box::new(x)), c)
            }
            super::TypeExpression::PropertyAccess(x, property) => {
                let (x, c) = x.visit(v, c);

                v.type_expression(
                    super::TypeExpression::PropertyAccess(Box::new(x), property),
                    c,
                )
            }
            super::TypeExpression::Function(parameters, x) => {
                let (x, c) = x.visit(v, c);
                let (parameters, c) = v.visit_each(parameters, c);

                v.type_expression(super::TypeExpression::Function(parameters, Box::new(x)), c)
            }
        }
    }
}

impl Visit for raw::Parameter {
    type Result<T> = super::Parameter<T, T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let super::Parameter {
            name,
            value_type,
            default_value,
        } = self.0.value;
        let (value_type, c) = v.visit_option(value_type, c);
        let (default_value, c) = v.visit_option(default_value, c);

        v.parameter(
            super::Parameter {
                name,
                value_type,
                default_value,
            },
            c,
        )
    }
}

impl Visit for raw::Declaration {
    type Result<T> = super::Declaration<T, T, T, T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Declaration::TypeAlias { storage, value } => {
                let (value, c) = value.visit(v, c);

                v.declaration(super::Declaration::TypeAlias { storage, value }, c)
            }
            super::Declaration::Constant {
                storage,
                value_type,
                value,
            } => {
                let (value_type, c) = v.visit_option(value_type, c);
                let (value, c) = value.visit(v, c);

                v.declaration(
                    super::Declaration::Constant {
                        storage,
                        value_type,
                        value,
                    },
                    c,
                )
            }
            super::Declaration::Enumerated { storage, variants } => {
                let (variants, c) =
                    variants
                        .into_iter()
                        .fold((vec![], c), |(mut acc, c), (name, arguments)| {
                            let (arguments, c) = v.visit_each(arguments, c);

                            acc.push((name, arguments));

                            (acc, c)
                        });

                v.declaration(super::Declaration::Enumerated { storage, variants }, c)
            }
            super::Declaration::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                let (parameters, c) = v.visit_each(parameters, c);
                let (body_type, c) = v.visit_option(body_type, c);
                let (body, c) = body.visit(v, c);

                v.declaration(
                    super::Declaration::Function {
                        storage,
                        parameters,
                        body_type,
                        body,
                    },
                    c,
                )
            }
            super::Declaration::View {
                storage,
                parameters,
                body,
            } => {
                let (parameters, c) = v.visit_each(parameters, c);
                let (body, c) = body.visit(v, c);

                v.declaration(
                    super::Declaration::View {
                        storage,
                        parameters,
                        body,
                    },
                    c,
                )
            }
            super::Declaration::Module { storage, value } => {
                let (value, c) = value.visit(v, c);

                v.declaration(super::Declaration::Module { storage, value }, c)
            }
        }
    }
}

impl Visit for raw::Import {
    type Result<T> = super::Import;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let super::Import {
            source,
            path,
            alias,
        } = self.0.value;

        v.import(
            super::Import {
                source,
                path,
                alias,
            },
            c,
        )
    }
}

impl Visit for raw::Module {
    type Result<T> = super::Module<T, T>;

    type Visitor<'a, T, C> = &'a Visitor<'a, T, C>
    where
        T: 'a, C: 'a;

    fn visit<Output, Context>(
        self,
        v: Self::Visitor<'_, Output, Context>,
        c: Context,
    ) -> (Output, Context) {
        let super::Module {
            imports,
            declarations,
        } = self.0.value;
        let (imports, c) = v.visit_each(imports, c);
        let (declarations, c) = v.visit_each(declarations, c);

        v.module(
            super::Module {
                imports,
                declarations,
            },
            c,
        )
    }
}
