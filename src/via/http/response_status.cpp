//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2019 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/response_status.hpp"
#include "via/http/character.hpp"

namespace via
{
  namespace http
  {
    namespace response_status
    {
    //////////////////////////////////////////////////////////////////////////
    const std::string_view reason_phrase(code status_code) noexcept
    {
      switch(status_code)
      {
      // Informational 1xx
      case code::CONTINUE:                      return REASON_CONTINUE;
      case code::SWITCHING_PROTOCOLS:           return REASON_SWITCHING_PROTOCOLS;

      // Successful 2xx
      case code::OK:                            return REASON_OK;
      case code::CREATED:                       return REASON_CREATED;
      case code::ACCEPTED:                      return REASON_ACCEPTED;
      case code::NON_AUTHORITATIVE:             return REASON_NON_AUTHORITATIVE;
      case code::NO_CONTENT:                    return REASON_NO_CONTENT;
      case code::RESET_CONTENT:                 return REASON_RESET_CONTENT;
      case code::PARTIAL_CONTENT:               return REASON_PARTIAL_CONTENT;

      // Redirection 3xx
      case code::MULTIPLE_CHOICES:              return REASON_MULTIPLE_CHOICES;
      case code::MOVED_PERMANENTLY:             return REASON_MOVED_PERMANENTLY;
      case code::FOUND:                         return REASON_FOUND;
      case code::SEE_OTHER:                     return REASON_SEE_OTHER;
      case code::NOT_MODIFIED:                  return REASON_NOT_MODIFIED;
      case code::USE_PROXY:                     return REASON_USE_PROXY;
      case code::TEMPORARY_REDIRECT:            return REASON_TEMPORARY_REDIRECT;
      case code::PERMANENT_REDIRECT:            return REASON_PERMANENT_REDIRECT;

      // Client Error 4xx
      case code::BAD_REQUEST:                   return REASON_BAD_REQUEST;
      case code::UNAUTHORISED:                  return REASON_UNAUTHORISED;
      case code::PAYMENT_REQUIRED:              return REASON_PAYMENT_REQUIRED;
      case code::FORBIDDEN:                     return REASON_FORBIDDEN;
      case code::NOT_FOUND:                     return REASON_NOT_FOUND;
      case code::METHOD_NOT_ALLOWED:            return REASON_METHOD_NOT_ALLOWED;
      case code::NOT_ACCEPTABLE:                return REASON_NOT_ACCEPTABLE;
      case code::PROXY_AUTHENTICATION_REQUIRED:
                                  return REASON_PROXY_AUTHENTICATION_REQUIRED;
      case code::REQUEST_TIMEOUT:               return REASON_REQUEST_TIMEOUT;
      case code::CONFLICT:                      return REASON_CONFLICT;
      case code::GONE:                          return REASON_GONE;
      case code::LENGTH_REQUIRED:               return REASON_LENGTH_REQUIRED;
      case code::PRECONDITION_FAILED:           return REASON_PRECONDITION_FAILED;
      case code::PAYLOAD_TOO_LARGE:             return REASON_PAYLOAD_TOO_LARGE;
      case code::REQUEST_URI_TOO_LONG:          return REASON_REQUEST_URI_TOO_LONG;
      case code::UNSUPPORTED_MEDIA_TYPE:       return REASON_UNSUPPORTED_MEDIA_TYPE;
      case code::REQUEST_RANGE_NOT_SATISFIABLE:
                                  return REASON_REQUEST_RANGE_NOT_SATISFIABLE;
      case code::EXPECTATION_FAILED:            return REASON_EXPECTATION_FAILED;
      case code::UPGRADE_REQUIRED:              return REASON_UPGRADE_REQUIRED;
      case code::PRECONDITION_REQUIRED:         return REASON_PRECONDITION_REQUIRED;
      case code::TOO_MANY_REQUESTS:             return REASON_TOO_MANY_REQUESTS;
      case code::REQUEST_HEADER_FIELDS_TOO_LARGE:
                                                return REASON_REQUEST_HEADER_FIELDS_TOO_LARGE;

      // Server Error 5xx
      case code::INTERNAL_SERVER_ERROR:         return REASON_INTERNAL_SERVER_ERROR;
      case code::NOT_IMPLEMENTED:               return REASON_NOT_IMPLEMENTED;
      case code::BAD_GATEWAY:                   return REASON_BAD_GATEWAY;
      case code::SERVICE_UNAVAILABLE:           return REASON_SERVICE_UNAVAILABLE;
      case code::GATEWAY_TIMEOUT:               return REASON_GATEWAY_TIMEOUT;
      case code::HTTP_VERSION_NOT_SUPPORTED:
                                     return REASON_HTTP_VERSION_NOT_SUPPORTED;
      case code::NETWORK_AUTHENTICATION_REQUIRED:
                                return REASON_NETWORK_AUTHENTICATION_REQUIRED;

      // Unknown Error Status Code
      default:                            return std::string_view();
      }
    }
    //////////////////////////////////////////////////////////////////////////

    }
  }
}
