//
// Copyright (c) 2013-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BEAST_TYPE_TRAITS_HPP
#define BEAST_TYPE_TRAITS_HPP

#include <beast/config.hpp>
#include <beast/core/detail/type_traits.hpp>
#include <boost/asio/buffer.hpp>
#include <type_traits>

namespace beast {

//------------------------------------------------------------------------------
//
// Buffer concepts
//
//------------------------------------------------------------------------------

/** Determine if `T` meets the requirements of @b ConstBufferSequence.

    @par Example
    Use with `static_assert`:
    @code
        template<class ConstBufferSequence>
        void f(ConstBufferSequence const& buffers)
        {
            static_assert(is_const_buffer_sequence<ConstBufferSequence>::value,
                "ConstBufferSequence requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class ConstBufferSequence>
        typename std::enable_if<is_const_buffer_sequence<ConstBufferSequence>::value>::type
        f(ConstBufferSequence const& buffers);
    @endcode
*/
template<class T>
#if BEAST_DOXYGEN
struct is_const_buffer_sequence : std::integral_constant<bool, ...>
#else
struct is_const_buffer_sequence :
    detail::is_buffer_sequence<T, boost::asio::const_buffer>
#endif
{
};

/** Determine if `T` meets the requirements of @b MutableBufferSequence.

    @par Example
    Use with `static_assert`:
    @code
        template<class MutableBufferSequence>
        void f(MutableBufferSequence const& buffers)
        {
            static_assert(is_const_buffer_sequence<MutableBufferSequence>::value,
                "MutableBufferSequence requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class MutableBufferSequence>
        typename std::enable_if<is_mutable_buffer_sequence<MutableBufferSequence>::value>::type
        f(MutableBufferSequence const& buffers);
    @endcode
*/
template<class T>
#if BEAST_DOXYGEN
struct is_mutable_buffer_sequence : std::integral_constant<bool, ...>
#else
struct is_mutable_buffer_sequence :
    detail::is_buffer_sequence<T, boost::asio::mutable_buffer>
#endif
{
};

/** Determine if `T` meets the requirements of @b DynamicBuffer.

    @par Example
    Use with `static_assert`:
    @code
        template<class DynamicBuffer>
        void f(DynamicBuffer& buffer)
        {
            static_assert(is_dynamic_buffer<DynamicBuffer>::value,
                "DynamicBuffer requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class DynamicBuffer>
        typename std::enable_if<is_dynamic_buffer<DynamicBuffer>::value>::type
        f(DynamicBuffer const& buffer);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_dynamic_buffer : std::integral_constant<bool, ...> {};
#else
template<class T, class = void>
struct is_dynamic_buffer : std::false_type {};

template<class T>
struct is_dynamic_buffer<T, detail::void_t<decltype(
    std::declval<std::size_t&>() =
        std::declval<T const&>().size(),
    std::declval<std::size_t&>() =
        std::declval<T const&>().max_size(),
    std::declval<std::size_t&>() =
        std::declval<T const&>().capacity(),
    std::declval<T&>().commit(std::declval<std::size_t>()),
    std::declval<T&>().consume(std::declval<std::size_t>()),
        (void)0)> > : std::integral_constant<bool,
    is_const_buffer_sequence<
        typename T::const_buffers_type>::value &&
    is_mutable_buffer_sequence<
        typename T::mutable_buffers_type>::value &&
    std::is_same<typename T::const_buffers_type,
        decltype(std::declval<T const&>().data())>::value &&
    std::is_same<typename T::mutable_buffers_type,
        decltype(std::declval<T&>().prepare(
            std::declval<std::size_t>()))>::value
        >
{
};

// Special case for Boost.Asio which doesn't adhere to
// net-ts but still provides a read_size_helper so things work
template<class Allocator>
struct is_dynamic_buffer<
    boost::asio::basic_streambuf<Allocator>> : std::true_type
{
};
#endif

//------------------------------------------------------------------------------
//
// Handler concepts
//
//------------------------------------------------------------------------------

/** Determine if `T` meets the requirements of @b CompletionHandler.

    This metafunction will be equivalent to `std::true_type` if `T`
    meets the requirements for a completion handler callable with
    the given signature.

    @par Example
    @code
    struct handler
    {
        void operator()(error_code&);
    };

    static_assert(is_completion_handler<handler, void(error_code&)>::value,
        "Not a completion handler");
    @endcode
*/
template<class T, class Signature>
#if BEAST_DOXYGEN
using is_completion_handler = std::integral_constant<bool, ...>;
#else
using is_completion_handler = std::integral_constant<bool,
    std::is_copy_constructible<typename std::decay<T>::type>::value &&
    detail::is_invocable<T, Signature>::value>;
#endif

//------------------------------------------------------------------------------
//
// Stream concepts
//
//------------------------------------------------------------------------------

/** Determine if `T` has the `get_io_service` member.

    @par Example
    @code
    struct stream
    {
        boost::asio::io_service& get_io_service();
    };

    static_assert(has_get_io_service<stream>::value,
        "Missing get_io_service member");
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct has_get_io_service : std::integral_constant<bool, ...>{};
#else
template<class T, class = void>
struct has_get_io_service : std::false_type {};

template<class T>
struct has_get_io_service<T, beast::detail::void_t<decltype(
    detail::accept_rv<boost::asio::io_service&>(
        std::declval<T&>().get_io_service()),
    (void)0)>> : std::true_type {};
#endif

/** Returns `T::lowest_layer_type` if it exists, else `T`

    @par Example
    @code
    template<class Stream>
    struct stream_wrapper
    {
        using next_layer_type = typename std::remove_reference<Stream>::type;
        using lowest_layer_type = typename get_lowest_layer<stream_type>::type;
    };
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct get_lowest_layer;
#else
template<class T, class = void>
struct get_lowest_layer
{
    using type = T;
};

template<class T>
struct get_lowest_layer<T, detail::void_t<
    typename T::lowest_layer_type>>
{
    using type = typename T::lowest_layer_type;
};
#endif

/** Determine if `T` meets the requirements of @b AsyncReadStream.

    @par Example
    Use with `static_assert`:
    @code
        template<class AsyncReadStream>
        void f(AsyncReadStream& stream)
        {
            static_assert(is_async_read_stream<AsyncReadStream>::value,
                "AsyncReadStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class AsyncReadStream>
        typename std::enable_if<is_async_read_stream<AsyncReadStream>::value>::type
        f(AsyncReadStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_async_read_stream : std::integral_constant<bool, ...>{};
#else
template<class T, class = void>
struct is_async_read_stream : std::false_type {};

template<class T>
struct is_async_read_stream<T, detail::void_t<decltype(
    std::declval<T>().async_read_some(
        std::declval<detail::MutableBufferSequence>(),
        std::declval<detail::ReadHandler>()),
            (void)0)>> : std::integral_constant<bool,
    has_get_io_service<T>::value
        > {};
#endif

/** Determine if `T` meets the requirements of @b AsyncWriteStream.

    @par Example
    Use with `static_assert`:
    @code
        template<class AsyncWriteStream>
        void f(AsyncWriteStream& stream)
        {
            static_assert(is_async_write_stream<AsyncWriteStream>::value,
                "AsyncWriteStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class AsyncWriteStream>
        typename std::enable_if<is_async_write_stream<AsyncWriteStream>::value>::type
        f(AsyncWriteStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_async_write_stream : std::integral_constant<bool, ...>{};
#else
template<class T, class = void>
struct is_async_write_stream : std::false_type {};

template<class T>
struct is_async_write_stream<T, detail::void_t<decltype(
    std::declval<T>().async_write_some(
        std::declval<detail::ConstBufferSequence>(),
        std::declval<detail::WriteHandler>()),
            (void)0)>> : std::integral_constant<bool,
    has_get_io_service<T>::value
        > {};
#endif

/** Determine if `T` meets the requirements of @b SyncReadStream.

    @par Example
    Use with `static_assert`:
    @code
        template<class SyncReadStream>
        void f(SyncReadStream& stream)
        {
            static_assert(is_sync_read_stream<SyncReadStream>::value,
                "SyncReadStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class SyncReadStream>
        typename std::enable_if<is_sync_read_stream<SyncReadStream>::value>::type
        f(SyncReadStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_sync_read_stream : std::integral_constant<bool, ...>{};
#else
template<class T, class = void>
struct is_sync_read_stream : std::false_type {};

template<class T>
struct is_sync_read_stream<T, detail::void_t<decltype(
    std::declval<std::size_t&>() = std::declval<T>().read_some(
        std::declval<detail::MutableBufferSequence>()),
    std::declval<std::size_t&>() = std::declval<T>().read_some(
        std::declval<detail::MutableBufferSequence>(),
        std::declval<boost::system::error_code&>()),
            (void)0)>> : std::integral_constant<bool,
    has_get_io_service<T>::value
        > {};
#endif

/** Determine if `T` meets the requirements of @b SyncWriterStream.

    @par Example
    Use with `static_assert`:
    @code
        template<class SyncReadStream>
        void f(SyncReadStream& stream)
        {
            static_assert(is_sync_read_stream<SyncReadStream>::value,
                "SyncReadStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class SyncReadStream>
        typename std::enable_if<is_sync_read_stream<SyncReadStream>::value>::type
        f(SyncReadStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_sync_write_stream : std::integral_constant<bool, ...>{};
#else
template<class T, class = void>
struct is_sync_write_stream : std::false_type {};

template<class T>
struct is_sync_write_stream<T, detail::void_t<decltype(
    std::declval<std::size_t&>() = std::declval<T&>().write_some(
        std::declval<detail::ConstBufferSequence>()),
    std::declval<std::size_t&>() = std::declval<T&>().write_some(
        std::declval<detail::ConstBufferSequence>(),
        std::declval<boost::system::error_code&>()),
            (void)0)>> : std::integral_constant<bool,
    has_get_io_service<T>::value
        > {};
#endif

/** Determine if `T` meets the requirements of @b AsyncStream.

    @par Example
    Use with `static_assert`:
    @code
        template<class AsyncStream>
        void f(AsyncStream& stream)
        {
            static_assert(is_async_stream<AsyncStream>::value,
                "AsyncStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class AsyncStream>
        typename std::enable_if<is_async_stream<AsyncStream>::value>::type
        f(AsyncStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_async_stream : std::integral_constant<bool, ...>{};
#else
template<class T>
using is_async_stream = std::integral_constant<bool,
    is_async_read_stream<T>::value && is_async_write_stream<T>::value>;
#endif

/** Determine if `T` meets the requirements of @b SyncStream.


    @par Example
    Use with `static_assert`:
    @code
        template<class SyncStream>
        void f(SyncStream& stream)
        {
            static_assert(is_sync_stream<SyncStream>::value,
                "SyncStream requirements not met");
            ...
        }
    @endcode
    Use with `std::enable_if`
    @code
        template<class SyncStream>
        typename std::enable_if<is_sync_stream<SyncStream>::value>::type
        f(SyncStream& stream);
    @endcode
*/
#if BEAST_DOXYGEN
template<class T>
struct is_sync_stream : std::integral_constant<bool, ...>{};
#else
template<class T>
using is_sync_stream = std::integral_constant<bool,
    is_sync_read_stream<T>::value && is_sync_write_stream<T>::value>;
#endif

} // beast

#endif