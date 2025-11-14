(define-module (miny)
  #:export (miny)

  #:use-module (guix gexp)
  #:use-module (guix packages)
  #:use-module (guix git-download)
  #:use-module (guix build-system gnu)
  #:use-module ((guix licenses)
                #:prefix license:)
  #:use-module (gnu packages gl))

(define (git-not-ignored? file stat)
  (define old-cwd (getcwd))
  (chdir (dirname file))
  (define keep?
    (and (not (equal? file (canonicalize-path ".git")))
	 (with-output-to-file "/dev/null"
	   (lambda ()
	     (= 1 (status:exit-val (system* "git" "check-ignore" file)))))))
  (chdir old-cwd)
  keep?)

(define miny
  (package
    (name "miny")
    (version "0.6.0")
    (source (local-file "." name #:recursive? #t #:select? git-not-ignored?))
    (build-system gnu-build-system)
    (arguments
     (list
      #:phases #~(modify-phases %standard-phases
                   (delete 'configure)
		   (replace 'install
		     (lambda* (#:key outputs #:allow-other-keys)
		       (let* ((out (assoc-ref outputs "out"))
			      (bin (string-append out "/bin")))
			 (install-file "miny" bin)))))
      #:tests? #f))
    (inputs (list freeglut))
    (home-page "https://github.com/spacecamper/miny")
    (synopsis "Minesweeper")
    (description "Minesweeper")
    (license license:expat)))
miny
