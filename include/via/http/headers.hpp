#ifndef HEADERS_HPP_VIA_HTTPLIB_
#define HEADERS_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file headers.hpp
/// @brief Classes to parse and encode HTTP headers.
//////////////////////////////////////////////////////////////////////////////
#include "header_field.hpp"
#include <unordered_map>

namespace via
{
  namespace http
  {
    constexpr char COOKIE[]   {"cookie"};
    constexpr char IDENTITY[] {"identity"};
    constexpr char CLOSE[]    {"close"};
    constexpr char CONTINUE[] {"100-continue"};

    /// @enum Rx is the receiver parsing state and is valid for both the
    /// request and response receivers.
    enum Rx
    {
      RX_INVALID,         ///< the message is invalid
      RX_EXPECT_CONTINUE, ///< the client expects a 100 Continue response
      RX_INCOMPLETE,      ///< the message requires more data
      RX_VALID,           ///< a valid request or response
      RX_CHUNK            ///< a valid chunk received
    };

    //////////////////////////////////////////////////////////////////////////
    /// @class field_line
    /// An HTTP header field.
    //////////////////////////////////////////////////////////////////////////
    class field_line
    {
    public:
      /// @enum Header the state of the header field line parser
      enum Header
      {
        HEADER_NAME,         ///< the header name field
        HEADER_VALUE_LS,     ///< the header value leading white space
        HEADER_VALUE,        ///< the header value
        HEADER_LF,           ///< the line feed (if any)
        HEADER_VALID,        ///< the header line is valid
        HEADER_ERROR_LENGTH, ///< the header line is longer than max_line_length_
        HEADER_ERROR_CRLF,   ///< strict_crlf_ is true and LF was received without CR
        HEADER_ERROR_WS      ///< the whitespace is longer than max_whitespace_
      };

    private:

      /// Parser parameters
      bool           strict_crlf_;     ///< enforce strict parsing of CRLF
      unsigned char  max_whitespace_;  ///< the max no of consectutive whitespace characters.
      unsigned short max_line_length_; ///< the max length of a field line

      /// Field information
      std::string   name_;     ///< the field name (lower case)
      std::string   value_;    ///< the field value
      size_t        length_;   ///< the length of the header line in bytes
      size_t        ws_count_; ///< the current whitespace count
      Header        state_;    ///< the current parsing state

      /// Parse an individual character.
      /// @param c the current character to be parsed.
      /// @retval state the current state of the parser.
      bool parse_char(char c)
      {
        // Ensure that the overall header length is within limitts
        if (++length_ > max_line_length_)
          state_ = HEADER_ERROR_LENGTH;

        switch (state_)
        {
        case HEADER_NAME:
          if (std::isalpha(c) || ('-' == c))
            name_.push_back(static_cast<char>(std::tolower(c)));
          else if (':' == c)
            state_ = HEADER_VALUE_LS;
          else
            return false;
          break;

        case HEADER_VALUE_LS:
          // Ignore leading whitespace
          if (is_space_or_tab(c))
            // but only upto to a limit!
            if (++ws_count_ > max_whitespace_)
            {
              state_ = HEADER_ERROR_WS;
              return false;
            }
            else
              break;
          else
            state_ = HEADER_VALUE;
          [[fallthrough]]; // intentional fall-through

        case HEADER_VALUE:
          // The header line should end with an \r\n...
          if (!is_end_of_line(c))
            value_.push_back(c);
          else if ('\r' == c)
            state_ = HEADER_LF;
          else // ('\n' == c)
          {
            if (strict_crlf_)
            {
              state_ = HEADER_ERROR_CRLF;
              return false;
            }
            else
              state_ = HEADER_VALID;
          }
          break;

        case HEADER_LF:
          if ('\n' == c)
            state_ = HEADER_VALID;
          else
            return false;
          break;

        default:
          return false;
        }

        return true;
      }

    public:

      /// Constructor.
      /// Sets the parser parameters and all member variables to their initial
      /// state.
      /// @param strict_crlf enforce strict parsing of CRLF.
      /// @param max_whitespace the maximum number of consectutive whitespace
      /// characters allowed in a request: min 1, max 254.
      /// @param max_line_length the maximum length of an HTTP header field line:
      /// max 65534.
      explicit field_line(bool           strict_crlf,
                          unsigned char  max_whitespace,
                          unsigned short max_line_length) :
        strict_crlf_(strict_crlf),
        max_whitespace_(max_whitespace),
        max_line_length_(max_line_length),
        name_(""),
        value_(""),
        length_(0),
        ws_count_(0),
        state_(HEADER_NAME)
      {}

      /// clear the field_line.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        name_.clear();
        value_.clear();
        length_ = 0;
        ws_count_ = 0;
        state_ = HEADER_NAME;
      }

      /// swap member variables with another field_line.
      /// @param other the other field_line
      void swap(field_line& other) noexcept
      {
        name_.swap(other.name_);
        value_.swap(other.value_);
        std::swap(length_, other.length_);
        std::swap(ws_count_, other.ws_count_);
        std::swap(state_, other.state_);
      }

      /// Parse an individual http header field and extract the field name
      /// (transformed to lower case) and value.
      /// @retval iter an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the buffer.
      /// @return true if a valid HTTP header, false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while ((iter != end) && (HEADER_VALID != state_))
        {
          char c(static_cast<char>(*iter++));
          if (!parse_char(c))
            return false;
          else if (HEADER_VALID == state_)
          { // determine whether the next line is a continuation header
            if ((iter != end) && is_space_or_tab(*iter))
            {
              value_.push_back(' ');
              state_ = HEADER_VALUE_LS;
            }
          }
        }

        return (HEADER_VALID == state_);
      }

      /// Accessor for the field name.
      /// @return the field name (as a lower case string)
      const std::string& name() const noexcept
      { return name_; }

      /// Accessor for the field value.
      /// @return the field value in the same case that it was received in.
      const std::string& value() const noexcept
      { return value_; }

      /// Calculate the length of the header.
      size_t length() const noexcept
      { return name_.size() + value_.size(); }
    }; // class field_line

    //////////////////////////////////////////////////////////////////////////
    /// @class message_headers
    /// The collection of HTTP headers received with a request, response or a
    /// chunk (trailers).
    /// Note: the parse function converts the received field names into lower
    /// case before storing them in a unordered_map for efficient access.
    /// @see rx_request
    /// @see rx_response
    /// @see rx_chunk
    //////////////////////////////////////////////////////////////////////////
    class message_headers
    {
      /// Parser parameters
      unsigned short max_header_number_; ///< the max no of header fields
      size_t         max_header_length_; ///< the max cumulative length

      /// The HTTP message header fields.
      std::unordered_map<std::string, std::string> fields_;
      field_line field_; ///< the current field being parsed
      bool       valid_; ///< true if the headers are valid
      size_t     length_; ///< the length of the message headers

    public:

      /// Constructor.
      /// Sets the parser parameters and all member variables to their initial
      /// state.
      /// @param strict_crlf enforce strict parsing of CRLF.
      /// @param max_whitespace the maximum number of consectutive whitespace
      /// characters allowed in a request: min 1, max 254.
      /// @param max_line_length the maximum length of an HTTP header field line:
      /// max 65534.
      /// @param max_header_number the maximum number of HTTP header field lines:
      /// max 65534.
      /// @param max_header_length the maximum cumulative length the HTTP header
      /// fields: max 4 billion.
      explicit message_headers(bool           strict_crlf,
                               unsigned char  max_whitespace,
                               unsigned short max_line_length,
                               unsigned short max_header_number,
                               size_t         max_header_length) :
        max_header_number_(max_header_number),
        max_header_length_(max_header_length),
        fields_(),
        field_(strict_crlf, max_whitespace, max_line_length),
        valid_(false),
        length_(0)
      {}

      /// Clear the message_headers.
      /// Sets all member variables to their initial state.
      void clear() noexcept
      {
        fields_.clear();
        field_.clear();
        valid_ = false;
        length_ = 0;
      }

      /// Swap member variables with another message_headers.
      /// @param other the other message_headers
      void swap(message_headers& other) noexcept
      {
        fields_.swap(other.fields_);
        field_.swap(other.field_);
        std::swap(valid_, other.valid_);
        std::swap(length_, other.length_);
      }

      /// Parse message_headers from a received request or response.
      /// @retval iter reference to an iterator to the start of the data.
      /// If valid it will refer to the next char of data to be read.
      /// @param end the end of the data buffer.
      /// @return true if parsed ok false otherwise.
      template<typename ForwardIterator>
      bool parse(ForwardIterator& iter, ForwardIterator end)
      {
        while (iter != end && !is_end_of_line(*iter))
        {
         // field_line field;
          if (!field_.parse(iter, end))
            return false;

          length_ += field_.length();
          add(field_.name(), field_.value());
          field_.clear();

          if ((length_ > max_header_length_)
           || (fields_.size() > max_header_number_))
            return false;
        }

        // Parse the blank line at the end of message_headers and
        // chunk trailers
        if (iter == end || !is_end_of_line(*iter))
          return false;

        // allow \r\n or just \n
        if ('\r' == *iter)
          ++iter;

        if ((iter == end) || ('\n' != *iter))
           return false;

        ++iter;
        valid_ = true;
        return valid_;
      }

      /// Add a header to the collection.
      /// @param name the field name (in lower case)
      /// @param value the field value.
      void add(std::string_view name, std::string_view value)
      {
        std::unordered_map<std::string, std::string>::iterator iter
          (fields_.find(name.data()));
        // if the field name was found previously
        if (iter != fields_.end())
        {
          char separator((name.find(COOKIE) != std::string::npos) ? ';' : ',');
          iter->second.append({separator});
          iter->second.append(value);
        }
        else
          fields_.insert(std::unordered_map<std::string, std::string>::value_type
                               (name, value));
      }

      /// Find the value for a given header name.
      /// Note: the name must be in lowercase for received message_headers.
      /// @param name the name of the header.
      /// @return the value, blank if not found
      std::string_view find(std::string_view name) const
      {
        std::unordered_map<std::string, std::string>::const_iterator iter
          (fields_.find(name.data()));
        return (iter != fields_.end()) ? iter->second : std::string_view();
      }

      /// Find the value for a given header id.
      /// @param field_id the id of the header.
      /// @return the value, blank if not found
      std::string_view find(header_field::id field_id) const
      { return find(header_field::lowercase_name(field_id)); }

      /// If there is a Content-Length field return its size.
      /// @return the value of the Content-Length field or
      /// -1 if it was invalid.
      /// May also return zero if it was not found.
      std::ptrdiff_t content_length() const noexcept
      {
        // Find whether there is a content length field.
        auto content_length(find(header_field::LC_CONTENT_LENGTH));
        return (content_length.empty()) ? 0 : from_dec_string(content_length);
      }

      /// Whether Chunked Transfer Coding is applied to the message.
      /// @return true if there is a transfer-encoding header and it does
      /// NOT contain the keyword "identity". See RFC2616 section 4.4 para 2.
      bool is_chunked() const
      {
        // Find whether there is a transfer encoding header.
        std::string xfer_encoding(find(header_field::LC_TRANSFER_ENCODING));
        if (xfer_encoding.empty())
          return false;

        std::transform(xfer_encoding.begin(), xfer_encoding.end(),
                       xfer_encoding.begin(), ::tolower);
        // Note: is transfer encoding if "identity" is NOT found.
        return (xfer_encoding.find(IDENTITY) == std::string::npos);
      }

      /// Whether the connection should be closed after the response.
      /// @return true if there is a Connection: close header, false otherwise
      bool close_connection() const
      {
        // Find whether there is a connection header.
        std::string connection(find(header_field::LC_CONNECTION));
        if (connection.empty())
          return false;

        std::transform(connection.begin(), connection.end(),
                       connection.begin(), ::tolower);
        return (connection.find(CLOSE) != std::string::npos);
      }

      /// Whether the client expects a "100-continue" response.
      /// @return true if there is an Expect: 100-continue header, false
      /// otherwise
      bool expect_continue() const
      {
        // Find whether there is a expect header.
        std::string expect(find(header_field::LC_EXPECT));
        if (expect.empty())
          return false;

        std::transform(expect.begin(), expect.end(),
                       expect.begin(), ::tolower);
        return (expect.find(CONTINUE) != std::string::npos);
      }

      /// Accessor for the valid flag.
      /// @return the valid flag.
      bool valid() const noexcept
      { return valid_; }

      // @return headers as a map
      const std::unordered_map<std::string, std::string>& fields() const
      { return fields_; }
      
      /// Output the message_headers as a string.
      /// Note: it is NOT terminated with an extra CRLF tso that it parses
      /// the are_headers_split function.
      /// @return a string containing all of the message_headers.
      std::string to_string() const
      {
        std::string output;
        for (std::unordered_map<std::string, std::string>::const_iterator
             iter(fields_.begin()); iter != fields_.end(); ++iter)
          output += header_field::to_header(iter->first, iter->second);

        return output;
      }
    };

    /// A function to determine whether the header string contains an extra
    /// CRLF pair, which could cause HTTP message spliting.
    inline bool are_headers_split(std::string_view headers) noexcept
    {
      char prev('0');
      char pprev('0');

      if (!headers.empty())
      {
        auto iter(headers.cbegin());
        for(; iter != headers.cend(); ++iter)
        {
          if (*iter == '\n')
          {
            if (prev == '\n')
              return true;
            else if ((prev == '\r') && (pprev == '\n'))
              return true;
          }

          pprev = prev;
          prev = *iter;
        }
      }

      return false;
    }
  }
}

#endif
