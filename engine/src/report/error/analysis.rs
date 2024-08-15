use super::{
    code::ToCode, CodeFrame, Display, ErrorCode, ErrorContext, ErrorDisplay, ErrorDisplayBuilder,
};
use crate::report::example;
use kore::{color::Highlight, str};

impl ToCode for analyze::Error {
    fn to_code(&self) -> ErrorCode {
        match self {
            Self::NotInferrable(_) => ErrorCode::NOT_INFERRABLE,
            Self::NotFound(_) => ErrorCode::NOT_FOUND,
            Self::VariantNotFound(..) => ErrorCode::VARIANT_NOT_FOUND,
            Self::DeclarationNotFound(..) => ErrorCode::DECLARATION_NOT_FOUND,
            Self::NotIndexable(..) => ErrorCode::NOT_INDEXABLE,
            Self::PropertyNotFound(..) => ErrorCode::PROPERTY_NOT_FOUND,
            Self::DuplicateProperty(_) => ErrorCode::DUPLICATE_PROPERTY,
            Self::NotSpreadable(_) => ErrorCode::NOT_SPREADABLE,
            Self::UntypedParameter(_) => ErrorCode::UNTYPED_PARAMETER,
            Self::DefaultValueRejected(_) => ErrorCode::DEFAULT_VALUE_REJECTED,
            Self::NotCallable(_) => ErrorCode::NOT_CALLABLE,
            Self::UnexpectedArgument(_) => ErrorCode::UNEXPECTED_ARGUMENT,
            Self::MissingArgument(_) => ErrorCode::MISSING_ARGUMENT,
            Self::ArgumentRejected(..) => ErrorCode::ARGUMENT_REJECTED,
            Self::NotRenderable(_) => ErrorCode::NOT_RENDERABLE,
            Self::InvalidComponent(_) => ErrorCode::INVALID_COMPONENT,
            Self::ComponentTypo(..) => ErrorCode::COMPONENT_TYPO,
            Self::InvalidAttributes(_) => ErrorCode::INVALID_ATTRIBUTES,
            Self::UnexpectedAttribute(_) => ErrorCode::UNEXPECTED_ATTRIBUTE,
            Self::MissingAttribute(_) => ErrorCode::MISSING_ATTRIBUTE,
            Self::AttributeRejected(..) => ErrorCode::ATTRIBUTE_REJECTED,
            Self::BinaryOperationNotSupported(..) => ErrorCode::BINARY_OPERATION_NOT_SUPPORTED,
            Self::UnaryOperationNotSupported(..) => ErrorCode::UNARY_OPERATION_NOT_SUPPORTED,
            Self::UnexpectedKind(..) => ErrorCode::UNEXPECTED_KIND,
        }
    }
}

impl<'a> Display<'a> for analyze::Error {
    type Context = (&'a lang::CanonicalId, &'a ErrorContext);

    fn display(
        &'a self,
        (
            id,
            ErrorContext {
                root_dir,
                modules,
                nodes,
                types,
            },
        ): Self::Context,
    ) -> ErrorDisplay<'a> {
        let module = modules.get(&id.0);
        let range = nodes.get(id);

        let builder = |title, description| {
            let builder = ErrorDisplayBuilder::new(self.to_code(), title, description);

            match (module, range) {
                (Some((link, text)), Some(range)) => builder.code_frame(CodeFrame {
                    root_dir,
                    link,
                    source: text,
                    range: *range,
                    color: true,
                }),
                _ => builder,
            }
        };

        match self {
            Self::NotInferrable(_) => builder(
                "Not Inferrable",
                str!("the type of this expression could not be inferred from other types"),
            )
            .build(),

            Self::NotFound(name) => builder(
                "Not Found",
                format!(
                    "This expression references a variable named {} which does not exist.",
                    name.error()
                ),
            )
            .suggestion(str!(
                "Check the spelling of the variable name or declare a new variable."
            ))
            .example(example::local_variables(name))
            .build(),

            // TODO: use a two-target code frame
            Self::VariantNotFound(_, variant) => builder(
                "Variant Not Found",
                format!(
                    "{} does not have a variant named {}.",
                    "This enumerated type".highlight(),
                    variant.error()
                ),
            )
            .build(),

            // TODO: use a two-target code frame
            Self::DeclarationNotFound(_, name) => builder(
                "Declaration Not Found",
                format!(
                    "This module does not contain a declaration named {}.",
                    name.error()
                ),
            )
            .build(),

            // TODO: use a two-target code frame
            Self::NotIndexable(_, name) => builder(
                "Not Indexable",
                // TODO: fix this error message
                format!(
                    "The property {} cannot be accessed because this type of value does not named",
                    name.error()
                ), // format!("the property {} cannot be accessed because this type of value does not support named properties", name.error())
            )
            .build(),

            // TODO: use a two-target code frame
            Self::PropertyNotFound(_, name) => builder(
                "Property Not Found",
                format!("a property with the name {} was not found", name.error()),
            )
            .build(),

            Self::DuplicateProperty(name) => {
                // caused the whole file to not format when inlined
                let description = format!(
                    "This expression contains two or more properties named {}. All property names must be unique.",
                    name.error(),
                );

                builder("Duplicate Property", description)
                    .suggestion(format!(
                        "Remove or rename any repeated {} properties to avoid conflict.",
                        name.highlight()
                    ))
                    .example(example::object_properties(
                        &format!("{}_0", name),
                        &format!("{}_1", name),
                    ))
                    .build()
            }

            // TODO: use a two-target code frame
            Self::NotSpreadable(_) => builder(
                "Not Spreadable",
                str!("only object-like types can be spread"),
            )
            .build(),

            Self::UntypedParameter(name) => builder(
                "Untyped Parameter",
                format!(
                    "The parameter {} is missing a type annotation.",
                    name.error()
                ),
            )
            .suggestion(format!(
                "Add an annotation describing the type of the {} parameter.",
                name.highlight()
            ))
            .example(example::parameter_types())
            .build(),

            // TODO: use a two-target code frame
            Self::DefaultValueRejected(_) => {
                // caused the whole file to not format when inlined
                let description = str!("The type of the default value does not match the annotated type of the parameter.");

                builder("Default Value Rejected", description).build()
            }

            // TODO: use a two-target code frame
            Self::NotCallable(_) => builder(
                "Not Callable",
                str!("this expression is not a function and cannot be called"),
            )
            .build(),

            // TODO: use a two-target code frame
            Self::UnexpectedArgument(_) => builder(
                "Unexpected Argument",
                str!("this function call expects fewer arguments than were provided"),
            )
            .build(),

            Self::MissingArgument(_) => builder(
                "Missing Argument",
                str!("this function call expects an additional argument that was not provided"),
            )
            .build(),

            Self::ArgumentRejected(_, _) => builder(
                "Argument Rejected",
                str!(
                    "this argument did not match the expected type based on the function signature"
                ),
            )
            .build(),

            Self::NotRenderable(_) => builder(
                "Not Renderable",
                str!("this expression is not a component and cannot be rendered"),
            )
            .build(),

            Self::InvalidComponent(name) => builder(
                "Invalid Component",
                format!(
                    "the variable {} does not reference a valid component type",
                    name.error()
                ),
            )
            .build(),

            Self::ComponentTypo(start_tag, end_tag) => builder(
                "Component Typo",
                format!(
                    "The start {} and end {} tags of this component do not match.",
                    format!("<{start_tag}>").error(),
                    format!("</{end_tag}>").error(),
                ),
            )
            .suggestion(str!("Change the end tag to match the start tag."))
            .example(example::open_component(start_tag))
            .build(),

            Self::InvalidAttributes(_) => builder("Invalid Attributes", format!("")).build(),

            Self::UnexpectedAttribute(name) => builder(
                "Unexpected Attribute",
                format!(
                    "this component does not accept an attribute named {}",
                    name.error()
                ),
            )
            .build(),

            Self::MissingAttribute(_) => builder(
                "Missing Attribute",
                str!("this component expected an attribute that was not provided"),
            )
            .build(),

            Self::AttributeRejected(_, _) => {
                // caused the whole file to not format when inlined
                let description = str!("this attribute did not match the expected type based on the component signature");

                builder("Attribute Rejected", description).build()
            }

            Self::BinaryOperationNotSupported(op, _, _) => builder(
                "Binary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the arguments provided",
                    op.to_string().highlight()
                ),
            )
            .build(),

            Self::UnaryOperationNotSupported(op, _) => builder(
                "Unary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the argument provided",
                    op.to_string().highlight()
                ),
            )
            .build(),

            Self::UnexpectedKind(_, kind) => builder(
                "Unexpected Kind",
                format!(
                    "this expression should be a {} but instead found a {}",
                    kind.to_string().success(),
                    kind.invert().to_string().error()
                ),
            )
            .build(),
        }
    }
}
